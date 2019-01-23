//  Parametric Moddel Rocket Component Library
//  by Jonathan Nobels, 2018
//
//  I dedicate any and all copyright interest in this software to the public domain.
//  I make this dedication for the benefit of the public at large and to the
//  detriment of my heirs and successors. I intend this dedication to be an overt
//  act of relinquishment in perpetuity of all present and future rights to this
//  software under copyright law. See <http://unlicense.org> for more info.
//

include<threads.scad>

//Tube Sytles
TUBE_15MT = 1; //1.5" Mailing Tube
TUBE_25MT = 2;     //2.5" Mailing Tube
TUBE_30MT = 3;     //3.0" Mailing Tube
TUBE_22TW = 4;     //2.2" Thick Wall (Rocketarium)
TUBE_25TW = 5;     //2.5" Thick Wall (Rocketaruim)
TUBE_BT80 = 6;     //Estes BT80 (2.5")
TUBE_BT60 = 7;     //Estes BT60
TUBE_BT50 = 8;     //Estes BT50
//-----------------
TUBE_TYPE = TUBE_22TW;

//Motor Tube Styles
MTUBE_29MM = 1;
MTUBE_BT50 = 2;
//-------------------
MOTOR_TUBE_TYPE = MTUBE_29MM;

$fn = 50;

MOTOR_LEN = 120; //Motor length.  Total len is MOTOR_LEN+RET_TNK
RET_THK = 3.5;   //Retainer thickness
WALL_THK = 1.5;  //1.5 default
TC_LEN = 20;     //Tail cone length
TC_CUTOFF = 12;
THREAD_LEN = 12.5;    //Length of the retainer threads
TOL = 0;              //Fit tolerance for retainers and cetnering ring
LUG_LEN = 20;         //Launch Lug Length
LL_BASE_OFFSET = 5.5; //Increase/decrease to change the lug base thickness

NUM_FINS = 4;         //3,4,5
FIN_THICKNESS = 3.35; //Fin Thickness
FIN_RET_THK = 2.0;    //Fin guide thickness

BOOSTER_ID = 22.0;
BOOSTER_T_ID = 24.0;
BOOSTER_OD = 25.0;
PORT_LEN = 40.0;
MOUNT_LEN = 30.0;

THREAD_DIAMETER = MOTOR_OD + 12;

if (NUM_FINS == 3)
{
  FIN_ANGLES = [ 0, 120, 240 ]; //Fin Angles.  None for no fin guides
}
else if (NUM_FINS == 5)
{
  FIN_ANGLES = [ 0, 72, 144, 216, 288 ]; //Fin Angles.  None for no fin guides
}
else if (NUM_FINS == 4)
{
  FIN_ANGLES = [ 0, 90, 180, 270 ]; //Fin Angles.  None for no fin guides
}

if (MOTOR_TUBE_TYPE == MTUBE_29MM)
{
  //29mm Motor Bute
  //O.D.: 1.233. I.D.: 1.147
  MOTOR_OD = 31.4; //29mm tube Motor Tube D.  Including some tolerance
}
else if (MOTOR_TUBE_TYPE == MTUBE_BT50)
{
  //24mm BT50
  MOTOR_OD = 0.99 * 25.4;
}

if (TUBE_TYPE == TUBE_15MT)
{
  //1.5" Mailing Tube
  TUBE_ID = 38.2;
  TUBE_OD = 41.2;
}
else if (TUBE_TYPE == TUBE_25MT)
{
  //2.5" Mailing Tube
  TUBE_ID = 63.65;
  TUBE_OD = 66.90;
}
else if (TUBE_TYPE == TUBE_30MT)
{
  //3.0" Mailing Tube
  TUBE_ID = 75.75;
  TUBE_OD = 78.75;
}
else if (TUBE_TYPE == TUBE_22TW)
{
  //Rocketarium thick wall 2.2" Tube
  //OD.: 2.26. I.D.: 2.14
  TUBE_OD = 2.26 * 25.4;
  TUBE_ID = 2.14 * 25.4;
}
else if (TUBE_TYPE == TUBE_25TW)
{
  //Rocketarium thick wall 2.5" Tube
  //O.D.: 2.638. I.D.: 2.558
  TUBE_OD = 2.638 * 25.4;
  TUBE_ID = 2.558 * 25.4;
}
else if (TUBE_TYPE == TUBE_BT50)
{
  //BT50 ID: 0.95", OD: 0.976"
  TUBE_ID = 0.95 * 25.4;
  TUBE_OD = 0.976 * 25.4;
}
else if (TUBE_TYPE == TUBE_BT60)
{
  //BT60: ID: 1.595", OD: 1.637
  TUBE_ID = 1.595 * 25.4;
  TUBE_OD = 1.637 * 25.4;
}
else if (TUBE_TYPE == TUBE_BT80)
{
  //BT80: OD: 2.6" ID: 2.558"
  TUBE_ID = 2.558 * 25.4;
  TUBE_OD = 2.60 * 25.4;
}

TUBE_ID = TUBE_ID - TOL;
MOTOR_OD = MOTOR_OD + TOL;

module cutting_jig()
{
  angle_marks_34 = [ 0, 90, 180, 270, 120, 240 ];
  thickness = 10;
  tolerance = 0.4;
  difference()
  {
    union()
    {
      cylinder(h = 14, r = TUBE_OD / 2 + thickness);
      cylinder(h = 20, r = TUBE_OD / 2 + 2);
    }
    cylinder(h = 80, r = TUBE_OD / 2 + tolerance, center = true);
    for (angle = angle_marks_34)
    {
      rotate([ 0, 0, angle ])
          cube([ 200, 3, 3 ], center = true);
    }
  }
}

module centering_ring()
{
  or = TUBE_ID / 2;
  ir = MOTOR_OD / 2;

  difference()
  {
    union()
    {
      l = (or -ir) * 1.9;
      h = 15;
      for (angle = FIN_ANGLES)
      {
        rotate([ 0, 0, angle ])
        {
          translate([ ir, FIN_THICKNESS, 8 ])
              cube([ l, FIN_THICKNESS, h ], center = true);
          translate([ ir, -FIN_THICKNESS, 8 ])
              cube([ l, FIN_THICKNESS, h ], center = true);
        }
      }

      cylinder(h = RET_THK, r = or, center = false);
    }
    translate([ 0, 0, -1 ])
    {
      cylinder(h = 30, r = ir, center = false);
      //cube(size = [ir*2 + 4, 6, RET_THK+8], center=true);
    }
  }
}

module tail_cone()
{
  difference()
  {
    or = TUBE_OD / 2;
    ir = MOTOR_OD / 2 + WALL_THK;
    mr = MOTOR_OD / 2 + WALL_THK;
    length = TC_LEN;
    cylinder(h = TC_LEN, r1 = or, r2 = ir, center = false);
    translate(0, 0, -1)
    {
      cylinder(h = length + 2, r = mr, center = false);
      cube(size = [ ir * 2 - 3, 6, TC_LEN * 2 + 2 ], center = true);
    }
    translate([ 0, 0, TC_CUTOFF * 2 ])
        cube([ or * 2, or *2, TC_CUTOFF * 2 ], center = true);
  }
}

module motor_retainer_inner()
{
  or = MOTOR_OD / 2 + 10;
  ir = MOTOR_OD / 2 + .3;
  difference()
  {
    //2.5 mm differene between this diameter and the other
    metric_thread(diameter = THREAD_DIAMETER, pitch = 2.2,
                  length = THREAD_LEN * 1.3, groove = false);
    cylinder(h = THREAD_LEN * 5 + 5, r = ir, center = true);
  }
}

module motor_retainer_outer()
{
  ir = MOTOR_OD / 2 + WALL_THK;
  length = THREAD_LEN + 6;
  startAngle = 0;
  angleInc = 360 / 15;
  union()
  {
    difference()
    {
      cylinder(h = length, r = MOTOR_OD / 2 + 11, center = false);
      metric_thread(diameter = THREAD_DIAMETER + 2.5, pitch = 2.2,
                    length = length + 1, groove = true);
      for (i = [0:14])
      {
        angle = angleInc * i;
        rotate([ 0, 0, angle ])
            translate([ MOTOR_OD / 2 + 11.1, 0, 0 ])
                cube([ 2, 4, 60 ], center = true);
      }
    }
    translate([ 0, 0, 0 ]) difference()
    {
      cylinder(h = 3, r = MOTOR_OD / 2 + 7, center = false);
      cylinder(h = 15, r = ir - 3.2, center = true);
    }
  }
}

module launch_lug()
{
  ll_size = 4.8; //1/4"
  ll_width = ll_size + 2;
  ll_base = 5;
  difference()
  {
    union()
    {
      cylinder(h = LUG_LEN, r = ll_width / 2, center = false);
      translate([ -ll_width / 2, -ll_width / 2, 0 ])
          cube([ ll_width, ll_width / 2, LUG_LEN ]);
      difference()
      {
        //translate([-ll_width/2-ll_base,-ll_width/2-2,0])
        //cube([ll_width+ll_base*2, 4,LUG_LEN]);
      }
    }
    cylinder(h = LUG_LEN * 2.2, r = ll_size / 2, center = true);
    translate([ 0, -TUBE_OD / 2 - LL_BASE_OFFSET, 0 ])
        cylinder(h = LUG_LEN * 2.2, r = TUBE_OD / 2, center = true);
  }
}

module booster_port()
{
  translate([ 0, 0, 12 ])
  {
    difference()
    {
      union()
      {
        translate([ -12, 0, 0 ])
            cube([ 24, 18, PORT_LEN ], center = false);
        cylinder(r = BOOSTER_OD / 2, h = PORT_LEN, center = false);
        translate([ 0, 0, -12 ])
            cylinder(r = BOOSTER_T_ID / 2, h = PORT_LEN, center = false);
        translate([ 0, 0, PORT_LEN ])
            cone_ogive_tan_blunted(R_nose = 4, R = BOOSTER_OD * .5,
                                   L = PORT_LEN * 1.1, s = 100);
        translate([ -10, 0, 3 ])
            cube([ 20, 30, PORT_LEN * 1.0 ], center = false);
      }
      cylinder(r = BOOSTER_ID / 2, h = PORT_LEN * 2.1, center = true);
      translate([ -10, 0, 3 ])
          cube([ 20, 30, PORT_LEN * .7 ], center = false);
      translate([ 0, TUBE_OD + 16, 0 ])
          cylinder(r = TUBE_OD, h = PORT_LEN * 2.2, center = true);
      translate([ 0, 0, PORT_LEN ])
          cone_ogive_tan_blunted(R_nose = 4, R = BOOSTER_OD * .45,
                                 L = PORT_LEN * 1.0, s = 100);
    }
    difference()
    {
      translate([ -10, 15, 3 ])
          cube([ 20, 10, PORT_LEN * .7 ], center = false);
      translate([ -8, 0, 5 ])
          cube([ 16, 28, PORT_LEN * .6 ], center = false);
    }
  }
}

module booster_mount()
{
  translate([ 0, 0, 0 ])
  {
    difference()
    {
      union()
      {
        translate([ -12, 0, 0 ]) cube([ 24, 18, PORT_LEN ], center = false);
        cylinder(r = BOOSTER_OD / 2 + 6, h = PORT_LEN, center = false);
        translate([ 0, 0, -12 ])
            cylinder(r = BOOSTER_T_ID / 2, h = PORT_LEN, center = false);
        translate([ 0, 0, PORT_LEN ])
            cone_ogive_tan_blunted(R_nose = 4, R = BOOSTER_OD * .5 + 6,
                                   L = 32, s = 100);
      }
      cylinder(r = BOOSTER_OD / 2, h = PORT_LEN * 5, center = true);
      translate([ 0, TUBE_OD + 16, 0 ])
          cylinder(r = TUBE_OD, h = PORT_LEN * 4.2, center = true);
    }
  }
}

module payload_adapter()
{
  WALL_THICKNESS = 1.6;
  PLATE_THICKNESS = 3.5;
  LENGTH = 70;
  union()
  {
    difference()
    {
      cylinder(r = TUBE_ID / 2, h = LENGTH, center = false);
      cylinder(r = TUBE_ID / 2 - WALL_THICKNESS, h = (LENGTH)*2.1, center = true);
    }
    difference()
    {
      translate([ 0, 0, 6 ])
          cylinder(r = TUBE_ID / 2, h = PLATE_THICKNESS, center = true);
      cylinder(r = 2, h = 30, center = true);
    }
  }
}

//translate([-TUBE_OD*.5,-10,0])launch_lug();
//translate([-TUBE_OD*.5,+10,0])launch_lug();
//launch_lug();

//translate([-TUBE_ID-4, 0, 0]) tail_cone();
//dual_mount();
//engine_mount_no_tube();
//centering_ring();
translate([ 0, 30, 0 ]) rotate([ 0, 0, -90 ]) motor_retainer_inner();
translate([ 0, -30, 0 ]) color([ 1, 0, 0 ]) motor_retainer_outer();

//centering_ring();

//Simple sleeve and bulkhead
//payload_adapter();

//tail_cone();
//cutting_jig();

//booster_port();
//booster_mount();

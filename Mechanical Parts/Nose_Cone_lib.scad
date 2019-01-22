// Parametric Haack Series or Ogive Nose Cone
// 2 or 3 piece with removable tip and sleeve
//
//  by Jonathan Nobels, 2017
//
//  I dedicate any and all copyright interest in this software to the public domain.
//  I make this dedication for the benefit of the public at large and to the
//  detriment of my heirs and successors. I intend this dedication to be an overt
//  act of relinquishment in perpetuity of all present and future rights to this
//  software under copyright law. See <http://unlicense.org> for more info.
//


$fn=120;

// 1.5" Mailing Tube
//TUBE_ID = 38.2;
//TUBE_OD = 41.2;

// 2.5" Mailing Tube
//TUBE_ID = 63.65;
//TUBE_OD = 66.90;  

// 3.0" Mailing Tube 
//TUBE_ID = 75.75;
//TUBE_OD = 78.75;

//Rocketarium thick wall 2.2" Tube
//OD.: 2.26. I.D.: 2.14
TUBE_OD = 2.26 *25.4;
TUBE_ID = 2.14 *25.4;

//Rocketarium thick wall 2.5" Tube
//O.D.: 2.638. I.D.: 2.558
//TUBE_OD = 2.638 *25.4;
//TUBE_ID = 2.558 *25.4;

//BT80: OD: 2.6" ID: 2.558"
//BT60: ID: 1.595", OD: 1.637
//BT50 ID: 0.95", OD: 0.976" 


LENGTH = 190;             //Overall length
TIP_LEN=60;               //Length of the removable tip 
TIP_RAD=4;                //Tip radiums
SHOULDER_LENGTH = 90;     //Length of the insert 

 //The radii and lengths here have to be customized for each length/od
 //Deriving the raduis at a given z position is left as an exercise for
 //the reader.
SLEEVE_OR=13.4;             //Tip insery sleeve outer radius
SLEEVE_IR=10;               //Tip insert sleeve inner radius
SLEEVE_LEN=20;              //Tip sleeve length
    
WALL_THICKNESS = 2.0;       //Cone and sleeve wall thickness

//Cone styles
PARABOLIC=1;
OGIVE=2;
HAACK=3;
ELLIPTICAL=4;

STYLE = OGIVE;

//Build the components - Uncomment the ones you need
//shelled_cone();
//translate([0,0,0])
//cone_bottom();
//translate([TUBE_OD/2 + 20,0,0])
//cone_tip();
//translate([-TUBE_OD - 20,0,0])
//translate([0,0,3.5])
avbay_insert();
//bulkhead();
//simple_insert();
//avbay_retainer();

// Components
module bulkhead()
{
  difference() {
    union(){
        cylinder(r=TUBE_ID/2, h=3.5, center=false);
        cylinder(r=TUBE_ID/2-WALL_THICKNESS-.1, h=6.0, center=false);
    }
    cylinder(r=1.5, h=20, center=true);
    for(x = [-TUBE_ID/2+8, TUBE_ID/2-8]) {
      translate([x,0,0]) {
       cylinder(r=3.0, h=20, center=true);
      }
    }
  }
}


module simple_insert()
{
  union(){
    difference(){
      cylinder(r=TUBE_ID/2, h=SHOULDER_LENGTH, center=false);
      cylinder(r=TUBE_ID/2-WALL_THICKNESS, h=(SHOULDER_LENGTH)*2.1, center=true);
    }
  }
}

module avbay_insert()
{
  union(){
    difference() {
      cylinder(r=TUBE_ID/2, h=SHOULDER_LENGTH, center=false);
      cylinder(r=TUBE_ID/2-WALL_THICKNESS, h=(SHOULDER_LENGTH)*2.1, center=true);
    }
    difference() {
      union() {
        translate([TUBE_ID/2-WALL_THICKNESS-11,-5,3.5]) {
            cube([12,10,14]);
        }
        translate([-TUBE_ID/2+WALL_THICKNESS-1,-5,3.5]) {
            cube([12,10,14]);
        }
      }
      for(x = [-TUBE_ID/2+8, TUBE_ID/2-8]) {
        translate([x,0,0]) {
         cylinder(r=3.0, h=70, center=true);
        }
      }
    }
  }  
}



module avbay_retainer()
{
  or = TUBE_ID / 2 - WALL_THICKNESS - .75;
  ir = or * .45;

  difference() {
    union() {
        translate([0,0,10]) {
          cube([or*1.9,9,20], center = true);
      }
      cylinder(h=3, r=or, center=false);
      
    }
    rotate([0,0,90])translate([TUBE_ID/2-WALL_THICKNESS-12,-5.5,-1]) {
        cube([12.8,11,24]);
    }
    rotate([0,0,90])translate([-TUBE_ID/2+WALL_THICKNESS-1,-5.5,-1]) {
         cube([12.8,11,24]);
    }
    translate([0,0,-1]) {
      //cylinder(h=50, r=ir, center=true);
      cube(size = [20,20,40], center=true);
    }
    translate([0,0,17])
     cube([51,2.2,25], center = true);
    translate([0,0,23]) 
       cube([46,40,28], center = true);
  }   
}

module solid_cone()
{
  if(STYLE == ELLIPTICAL) {
      cone_elliptical(R = TUBE_OD/2, L=LENGTH, s=180);
  }
  if(STYLE == OGIVE) {
      cone_ogive_tan_blunted(R = TUBE_OD/2, R_nose = TIP_RAD, L = LENGTH, s = 180);
  }
  if(STYLE == PARABOLIC) {
      cone_parabolic(R = TUBE_OD/2, L=LENGTH, s=180);
  }
  if(STYLE == HAACK){
      cone_haack(R = TUBE_OD/2, L = LENGTH, s = 180, R_nose=TIP_RAD);   
  }
}

module shelled_cone() 
{
  difference() {
  if(STYLE == ELLIPTICAL) {
   cone_elliptical(R = TUBE_OD/2, L=LENGTH, s=180);
   cone_elliptical(R = TUBE_OD/2 - WALL_THICKNESS, L=LENGTH-WALL_THICKNESS, s=180);
  }
  if(STYLE == OGIVE) {
   difference() { 
     cone_ogive_tan_blunted(R = TUBE_OD/2, R_nose = TIP_RAD, L = LENGTH, s = 90);
     cone_ogive_tan_blunted(R= TUBE_OD/2- WALL_THICKNESS, R_nose = TIP_RAD, L = LENGTH*.96, s = 90);
   }
  }
  if(STYLE == PARABOLIC) {
   cone_parabolic(R = TUBE_OD/2, L=LENGTH, s=100);
   cone_parabolic(R = TUBE_OD/2 - WALL_THICKNESS, L=LENGTH-WALL_THICKNESS, s=100);
  }
  if(STYLE == HAACK){
    cone_haack(R = TUBE_OD/2, L = LENGTH, s = 200, R_nose=TIP_RAD);
    cone_haack(R = TUBE_OD/2 - WALL_THICKNESS, L = LENGTH-WALL_THICKNESS, s = 180);
  }
  
  //Cut out inner sleeve
  cylinder(r=TUBE_ID/2, h = 20, center=true);
  }
}

module cone_bottom()
{  
  difference() {
    union() {
      shelled_cone();
      translate([0,0,LENGTH-TIP_LEN-SLEEVE_LEN]) difference(){
        cylinder(r=SLEEVE_OR,h=SLEEVE_LEN*2, center=false);
        cylinder(r=SLEEVE_IR,h=SLEEVE_LEN*4, center=true);
      }
    }
    translate([-SLEEVE_OR*2.5,-SLEEVE_OR*2.5,LENGTH-TIP_LEN])cube([SLEEVE_OR*5,SLEEVE_OR*5,TIP_LEN*2]);
  }
}

module cone_tip()
{ 
  translate([0,0,-(LENGTH-TIP_LEN-SLEEVE_LEN)])
  union() {
  difference() {
    solid_cone();
    translate([-TUBE_OD,-TUBE_OD,0])cube([TUBE_OD*2,TUBE_OD*2,LENGTH-TIP_LEN]);
  }
  translate([0,0,LENGTH-TIP_LEN-SLEEVE_LEN])
    cylinder(r=SLEEVE_IR-.4,h=SLEEVE_LEN, center=false);
  }
}

module cone_haack(C = 0, R = 5, L = 10, s = 100)
{
  // SEARS-HAACK BODY NOSE CONE:
  //
  // Parameters:
  // C = 1/3: LV-Haack (minimizes supersonic drag for a given L & V)
  // C = 0: LD-Haack (minimizes supersonic drag for a given L & D), also referred to as Von Kármán
  //
  // Formulae (radians):
  // theta = acos(1 - (2 * x / L));
  // y = (R / sqrt(PI)) * sqrt(theta - (sin(2 * theta) / 2) + C * pow(sin(theta),3));

  echo(str("SEARS-HAACK BODY NOSE CONE"));
  echo(str("C = ", C)); 
  echo(str("R = ", R)); 
  echo(str("L = ", L)); 
  echo(str("s = ", s)); 

  TORAD = PI/180;
  TODEG = 180/PI;

  inc = 1/s;

  rotate_extrude(convexity = 10, $fn = s)
    for (i = [1 : s]){
        x_last = L * (i - 1) * inc;
        x = L * i * inc;

        theta_last = TORAD * acos((1 - (2 * x_last/L)));
        y_last = (R/sqrt(PI)) * sqrt(theta_last - (sin(TODEG * (2*theta_last))/2) + C * pow(sin(TODEG * theta_last), 3));

        theta = TORAD * acos(1 - (2 * x/L));
        y = (R/sqrt(PI)) * sqrt(theta - (sin(TODEG * (2 * theta)) / 2) + C * pow(sin(TODEG * theta), 3));

        rotate([0, 0, -90]) polygon(points = [[x_last - L, 0], [x - L, 0], [x - L, y], [x_last - L, y_last]], convexity = 10);
    }
}
  
  

module cone_ogive_tan_blunted(R_nose = 2, R = 5, L = 10, s = 500)
{
    // SPHERICALLY BLUNTED TANGENT OGIVE
    //
    //
    
    echo(str("SPHERICALLY BLUNTED TANGENT OGIVE"));    
    echo(str("R_nose = ", R_nose)); 
    echo(str("R = ", R)); 
    echo(str("L = ", L)); 
    echo(str("s = ", s)); 
    
    rho = (pow(R,2) + pow(L,2)) / (2*R);
    
    x_o = L - sqrt(pow((rho - R_nose), 2) - pow((rho - R), 2));
    x_a = x_o - R_nose;
    y_t = (R_nose * (rho - R)) / (rho - R_nose);
    x_t = x_o - sqrt(pow(R_nose, 2)- pow(y_t, 2));
    
    TORAD = PI/180;
    TODEG = 180/PI;
        
    inc = 1/s;
        
    s_x_t = round((s * x_t) / L);
    
    alpha = TORAD * atan(R/L) - TORAD * acos(sqrt(pow(L,2) + pow(R,2)) / (2*rho));
    
    rotate_extrude(convexity = 10, $fn = s) union(){
        for (i=[s_x_t:s]){
    
            x_last = L * (i - 1) * inc;
            x = L * i * inc;
    
            y_last = sqrt(pow(rho,2) - pow((rho * cos(TODEG * alpha) - x_last),2)) + (rho * sin(TODEG * alpha));
    
            y = sqrt(pow(rho,2) - pow((rho * cos(TODEG * alpha) - x),2)) + (rho * sin(TODEG * alpha));
    
            rotate([0,0,-90])polygon(points = [[x_last-L,0],[x-L,0],[x-L,y],[x_last-L,y_last]], convexity = 10);
        }
    
        translate([0, L - x_o, 0]) difference(){
            circle(R_nose, $fn = s);
            translate([-R_nose, 0, 0]) square((2 * R_nose), center = true);
        }
    }
}  

module cone_parabolic(R = 5, L = 10, K = 1, s = 500)
{
// PARABOLIC NOSE CONE
//
// Formula: y = R * ((2 * (x / L)) - (K * pow((x / L),2)) / (2 - K);
//
// Parameters:
// K = 0 for cone
// K = 0.5 for 1/2 parabola
// K = 0.75 for 3/4 parabola
// K = 1 for full parabola

echo(str("PARABOLIC NOSE CONE"));
echo(str("R = ", R)); 
echo(str("L = ", L)); 
echo(str("K = ", K)); 
echo(str("s = ", s)); 
    
if (K >= 0 && K <= 1){

    inc = 1/s;

    rotate_extrude(convexity = 10, $fn = s)
    for (i = [1 : s]){
        
        x_last = L * (i - 1) * inc;
        x = L * i * inc;

        y_last = R * ((2 * ((x_last)/L)) - (K * pow(((x_last)/L), 2))) / (2 - K);
        y = R * ((2 * (x/L)) - (K * pow((x/L), 2))) / (2 - K);

        polygon(points = [[y_last, 0], [y, 0], [y, L - x], [y_last, L - x_last]], convexity = 10);
    }
} else echo(str("ERROR: K = ", K, ", but K must fall between 0 and 1 (inclusive)."));
}

module cone_elliptical(n = 0.5, R = 5, L = 10, s = 500){
// ELLIPTICAL NOSE CONE:
//
// Formula: y = R * sqrt(1 - pow((x / L), 2));

echo(str("ELLIPTICAL NOSE CONE"));    
echo(str("n = ", n)); 
echo(str("R = ", R)); 
echo(str("L = ", L)); 
echo(str("s = ", s)); 

inc = 1/s;

rotate_extrude(convexity = 10, $fn = s)
for (i = [1 : s]){

    x_last = L * (i - 1) * inc;
    x = L * i * inc;

    y_last = R * sqrt(1 - pow((x_last/L), 2));
    y = R * sqrt(1 - pow((x/L), 2));

    rotate([0,0,90]) polygon(points = [[0, y_last], [0, y], [x, y], [x_last, y_last]], convexity = 10);
}
}
  
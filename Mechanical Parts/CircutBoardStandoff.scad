
//Large Proto Board
SO_W=46;
SO_L=66;


//Small PCB
//SO_W=16;
//SO_L=76;

//4 Relay Board
//SO_W=48;
//SO_L=69;

W = SO_W+8;
L = SO_L+8;
H = 2;

SO_OD = 7;
SO_ID = 2.4;
SO_H = 10;
CO_RATIO = .66;

$fn=60;

base();

translate([-SO_W/2, -SO_L/2,0]) 
standoff();

translate([SO_W/2, -SO_L/2,0]) 
standoff();

translate([-SO_W/2, SO_L/2,0]) 
standoff();

translate([SO_W/2, SO_L/2,0]) 
standoff();


module base() {
   {
    difference() {
      translate([-W/2, -L/2,0])
      cube([W,L,H]);
      cube([W*CO_RATIO,L*CO_RATIO,H*2],center=true);
    }
  }
}
  
module standoff()
{
  difference() {
   cylinder(r=SO_OD/2, h=SO_H); 
   cylinder(r=SO_ID/2, h=SO_H*2); 
  } 
}

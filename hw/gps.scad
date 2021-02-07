////////////////////////////////////////////////////////////////////////////////
// gps.scad
////////////////////////////////////////////////////////////////////////////////

// https://github.com/MuellerA/Thingiverse.git#ac669d2e31545f4aca594795901329ed9a7b87eb
use <../../../3D Druck/Thingiverse/PCB/pcb.scad> ;

pcb = "all" ; // [top,bottom,button,all,testGps,testAnt]

// see measure.svg

gpsX = 36.5 ; // [15:0.1:45]
gpsY = 27.3 ; // [15:0.1:45]

antX = 26.5 ; // [5:0.1:40]
antY = 26.5 ; // [5:0.1:40]

/* [Hidden] */

nanoX  = 46.5 ;
nanoY  = 20.5 ;

minOX = 3 ;
minOY = 3 ;
dX    = 4 ;

minGpsOX = 9 ;

wallWidth = 0.8 ;
$fn = 40 ;

// calc total size excl outer wall

totTopX = 3*wallWidth + nanoX            + dX + antX + minOX ;
totBotX = 4*wallWidth + minGpsOX + gpsX  + dX + antX + minOX ;

totX = max(totTopX, totBotX) ;

totNanY = 2*wallWidth + 2*minOY + nanoY ;
totAntY = 2*wallWidth + 2*minOY + antY ;
totGpsY = 2*wallWidth + 2*minOY + gpsY ;

totY = max(totNanY, totAntY, totGpsY) ;

// calc offsets

offsetAntX = -antX/2 + totX - minOX - wallWidth ;
offsetGpsX = -gpsX/2 + totX - minOX - antX - dX - 3*wallWidth ;

////////////////////////////////////////////////////////////////////////////////

module base(height)
{
  translate([totX/2, 0, height/2])
    difference()
  {
    translate([0, 0, -wallWidth/2])
      cube([totX+2*wallWidth, totY+2*wallWidth, height+wallWidth], center=true) ;
    translate([0, 0, +wallWidth/2])
      cube([totX            , totY            , height+wallWidth], center=true) ;
  }
}

////////////////////////////////////////////////////////////////////////////////

module antenna()
{
  antZ = 4 ;

  difference()
  {
    translate([0, 0, antZ/2])
      cube([antX+2*wallWidth, antY+2*wallWidth, antZ], center=true) ;

    translate([0, 0, antZ/2])
      cube([antX            , antY          , 2*antZ], center=true) ;
    translate([-antX/2, 0, antZ/2])
      cube([8, 4, 2*antZ], center=true) ;
  }
}

////////////////////////////////////////////////////////////////////////////////

module gps()
{
  pcbHeight  =  1.2 ;
  wallWidth  =  0.8 ;
  railHeight =  2.0 ;
  railOffset =  8.0 ;

  clip = [ 3, 3.5 ] ;
  PcbHolder(gpsY, gpsX, pcbHeight, wallWidth, railHeight, railOffset,
            clip=clip, fingerHoleEnable=true) ;
}

////////////////////////////////////////////////////////////////////////////////

module hooks(height, top)
{
  w = wallWidth ;
  d = w/2 ;
  points =
    [
      [ [0, 0], [w, -w], [0, -2*w], [-w, -2*w], [-w, 0] ],
      [ [0, 0], [0, -2*w], [w+d, -2*w], [w+d, 0.5*w], [d, 1.5*w], [w+d, 2.5*w], [2*w+d, 2.2*w], [2*w+d, -3*w], [0, -6*w], [-w, -6*w], [-w, 0] ]
    ] ;

  module hook(points)
  {
    rotate([0, 0, 90])
      rotate([90, 0, 0])
      linear_extrude(height=3, center=true)
      polygon(points) ;
  }

  p1 = (top) ? points[0] : points[1] ;
  p2 = (top) ? points[1] : points[0] ;
  
  translate([+totX/2-5, -totY/2, height]) rotate([0, 0,   0]) hook(p1) ;
  translate([0        , -totY/2, height]) rotate([0, 0,   0]) hook(p2) ;
  translate([-totX/2+5, -totY/2, height]) rotate([0, 0,   0]) hook(p1) ;
  translate([+totX/2-5, +totY/2, height]) rotate([0, 0, 180]) hook(p1) ;
  translate([0        , +totY/2, height]) rotate([0, 0, 180]) hook(p2) ;
  translate([-totX/2+5, +totY/2, height]) rotate([0, 0, 180]) hook(p1) ;

  translate([+totX/2  , -totY/2+6  , height]) rotate([0, 0,  90]) hook(p1) ;
  translate([+totX/2  , +totY/2-6  , height]) rotate([0, 0,  90]) hook(p1) ;

  translate([-totX/2  , -totY/2+2.8, height]) rotate([0, 0, 270]) hook(p1) ;
  translate([-totX/2  , +totY/2-2.8, height]) rotate([0, 0, 270]) hook(p1) ;
}

////////////////////////////////////////////////////////////////////////////////

module top()
{
  pcbHeight  =  1.9 ;
  railHeight =  6.0 ;
  railOffset =  2.3 ;

  height = railHeight + pcbHeight + wallWidth + 1 ;
  
  dispLength = 23.9 ;
  dispWidth  = 13.5 ;
  dispOffset =  3.5 ;

  buttonD       = 3.0 ;
  buttonLoffset = nanoX / 2 - 8.9 ;
  buttonWoffset = 3.5 ;

  usbWidth  = 9.9 ;
  usbHeight = 3.9 ;
  usbOffset = 2.0 ;
  
  difference()
  {
    union()
    {
      // base
      base(height) ;

      // nano
      translate([nanoX/2, 0, 0])
      {
        // pcb frame
        rotate([0, 0, 90])
          PcbHolder(nanoX, nanoY, pcbHeight, wallWidth, railHeight, railOffset, clip = [10, 5]) ;

        translate([-dispLength/2+dispOffset-1.5*wallWidth, 0, 2/2]) cube([wallWidth, 20 , 2], center=true) ;

        // button
        bd = buttonD + 2 * 0.4 ;
        translate([-buttonLoffset, buttonWoffset, 0.51]) cylinder(d=bd+2*wallWidth, h=1, center=true) ;
        
        // reset
        translate([-buttonLoffset, -buttonWoffset, 0.51]) cylinder(d=bd+2*wallWidth, h=1, center=true) ;
      }

      // antenna
      translate([offsetAntX, 0, 0]) antenna() ;
    }

    union()
    {
      translate([nanoX/2, 0, 0])
      {
        // display
        translate([dispOffset, 0, 0]) cube([dispLength, dispWidth, 10], center=true) ;

        // button
        bd = buttonD + 2 * 0.4 ;
        translate([-buttonLoffset, buttonWoffset, 0]) cylinder(d=bd, h=10, center=true) ;

        // reset
        translate([-buttonLoffset, -buttonWoffset, 0]) cylinder(d=2, h=10, center=true) ;

        // usb
        translate([-nanoX/2, 0, usbHeight/2+usbOffset])
          cube([5, usbWidth, usbHeight], center=true) ;

        // sdcard
        translate([-nanoX/2, 0, railHeight + pcbHeight + 2])
          cube([5, 12, 4], center=true) ;
      }
    }
  }

  translate([totX/2, 0, 0])
    hooks(height, true) ;
}

////////////////////////////////////////////////////////////////////////////////

module bottom()
{
  sdcardX =  7 ;
  sdcardY = 12 ;
  
  height = 7 ;

  difference()
  {
    union()
    {
      // base
      base(height) ;

      // sdcard
      translate([(sdcardX)/2, 0, height/2])
        cube([sdcardX+2*wallWidth, sdcardY+2*wallWidth, height], center=true) ;

      // gps
      {
        translate([offsetGpsX, 0, 0])
          gps() ;
      }
      
      // antenna
      translate([offsetAntX, 0, 0]) antenna() ;
    }

    union()
    {
      // sdcard
      translate([(sdcardX)/2-wallWidth, 0, height/2])
        cube([sdcardX+2*wallWidth, sdcardY, height*2], center=true) ;
      translate([sdcardX+wallWidth/2, 0, height])
        cube([2*wallWidth, sdcardY, 2], center=true) ;
    }
  }   
  
  translate([totX/2, 0, 0])
    hooks(height, false) ;
}

////////////////////////////////////////////////////////////////////////////////

module button()
{
  cylinder(h=1, d=4.4) ;
  cylinder(h=6, d=2.5) ;
}

////////////////////////////////////////////////////////////////////////////////

if (pcb == "top")
  top() ;
else if (pcb == "bottom")
  bottom() ;
else if (pcb == "button")
  button() ;
else if (pcb == "all")
{
  translate([0, -totY/2 - 5, 0]) bottom() ;
  translate([0, +totY/2 + 5, 0]) top() ;
  translate([-10, 0, -wallWidth]) button() ;
}
else if (pcb == "testGps")
{
  translate([0, 0, -wallWidth/2])
    cube([gpsX+2*wallWidth, gpsY+2*wallWidth, wallWidth], center=true) ;
  gps() ;
}
else if (pcb == "testAnt")
{
  translate([0, 0, -wallWidth/2])
    cube([antX+2*wallWidth, antY+2*wallWidth, wallWidth], center=true) ;
  antenna() ;
}

////////////////////////////////////////////////////////////////////////////////
// EOF
////////////////////////////////////////////////////////////////////////////////

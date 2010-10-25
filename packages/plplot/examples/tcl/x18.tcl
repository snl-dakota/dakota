#----------------------------------------------------------------------------
# $Id: x18.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

proc x18 {{w loopback}} {

    matrix opt i 4 = {1, 0, 1, 0}
    matrix alt f 4 = {20.0, 35.0, 50.0, 65.0}
    matrix az  f 4 = {30.0, 40.0, 50.0, 60.0}

    set npts 1000
    set pi 3.14159265358979323846

    for {set k 0} {$k < 4} {incr k} {
	test_poly $w $k
    }

    matrix x f $npts
    matrix y f $npts
    matrix z f $npts

# From the mind of a sick and twisted physicist...

    for {set i 0} {$i < $npts} {incr i} {
	z $i = [expr -1. + 2. * $i / $npts]

# Pick one ... 

#	r    = 1. - ( (float) i / (float) NPTS );
	set r [z $i]

	x $i = [expr $r * cos( 2. * $pi * 6. * $i / $npts )]
	y $i = [expr $r * sin( 2. * $pi * 6. * $i / $npts )]
    }

    for {set k 0} {$k < 4} {incr k} {
	$w cmd pladv 0
	$w cmd plvpor 0.0 1.0 0.0 0.9
	$w cmd plwind -1.0 1.0 -0.9 1.1
	$w cmd plcol0 1
	$w cmd plw3d 1.0 1.0 1.0 -1.0 1.0 -1.0 1.0 -1.0 1.0 [alt $k], [az $k]
	$w cmd plbox3 "bnstu" "x axis" 0.0 0 \
	    "bnstu" "y axis" 0.0 0 \
	    "bcdmnstuv" "z axis" 0.0 0

	$w cmd plcol0 2

	if {[opt $k]} {
	    $w cmd plline3 $npts x y z
	} else {
	    $w cmd plpoin3 $npts x y z 1
	}

	$w cmd plcol0 3
	set title [format "#frPLplot Example 18 - Alt=%.0f, Az=%.0f" \
		       [alt $k] [az $k]]
	$w cmd plmtex t 1.0 0.5 0.5 "$title"
    }
    # Restore defaults
    $w cmd plcol0 1
}

proc test_poly {{w loopback} k} {

    matrix draw0 i 4 = { 1, 1, 1, 1 }
    matrix draw1 i 4 = { 1, 0, 1, 0 }
    matrix draw2 i 4 = { 0, 1, 0, 1 }
    matrix draw3 i 4 = { 1, 1, 0, 0 }

    set pi 3.14159265358979323846; set two_pi [expr 2. * $pi]

    matrix x f 5
    matrix y f 5
    matrix z f 5

    $w cmd pladv 0
    $w cmd plvpor 0.0 1.0 0.0 0.9
    $w cmd plwind -1.0 1.0 -0.9 1.1
    $w cmd plcol0 1
    $w cmd plw3d 1.0 1.0 1.0 -1.0 1.0 -1.0 1.0 -1.0 1.0 [alt $k] [az $k]
    $w cmd plbox3 "bnstu" "x axis" 0.0 0 \
	"bnstu" "y axis" 0.0 0 \
	"bcdmnstuv" "z axis" 0.0 0

    $w cmd plcol0 2

#define THETA(a) (two_pi * (a) /20.)
#define PHI(a)    (pi * (a) / 20.1)

#  x = r sin(phi) cos(theta)
#  y = r sin(phi) sin(theta)
#  z = r cos(phi)
#  r = 1 :=)

    for {set i 0} {$i < 20} {incr i} {
	for {set j 0} {$j < 20} {incr j} {
	    x 0 = [expr sin( $pi * $j / 20.1 ) * cos( $two_pi * $i / 20 )]
	    y 0 = [expr sin( $pi * $j / 20.1 ) * sin( $two_pi * $i / 20 )]
	    z 0 = [expr cos( $pi * $j / 20.1 )]

	    x 1 = [expr sin( $pi * ($j+1) / 20.1 ) * cos( $two_pi * $i / 20 )]
	    y 1 = [expr sin( $pi * ($j+1) / 20.1 ) * sin( $two_pi * $i / 20 )]
	    z 1 = [expr cos( $pi * ($j+1) / 20.1 )]

	    x 2 = [expr sin($pi * ($j+1) / 20.1) * cos($two_pi * ($i+1) / 20)]
	    y 2 = [expr sin($pi * ($j+1) / 20.1) * sin($two_pi * ($i+1) / 20)]
	    z 2 = [expr cos($pi * ($j+1) / 20.1)]

	    x 3 = [expr sin( $pi * $j / 20.1 ) * cos( $two_pi * ($i+1) / 20 )]
	    y 3 = [expr sin( $pi * $j / 20.1 ) * sin( $two_pi * ($i+1) / 20 )]
	    z 3 = [expr cos( $pi * $j / 20.1 )]

	    x 4 = [expr sin( $pi * $j / 20.1 ) * cos( $two_pi * $i / 20 )]
	    y 4 = [expr sin( $pi * $j / 20.1 ) * sin( $two_pi * $i / 20 )]
	    z 4 = [expr cos( $pi * $j / 20.1 )]

	    $w cmd plpoly3 5 x y z draw$k 1
	}
    }

    $w cmd plcol0 3
    $w cmd plmtex "t" 1.0 0.5 0.5 "unit radius sphere"
}

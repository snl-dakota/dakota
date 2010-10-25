#----------------------------------------------------------------------------
# $Id: x22.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

# Vector plot demo.

proc x22 {{w loopback}} {
    matrix arrow_x f 6 = {-0.5, 0.5, 0.3, 0.5, 0.3, 0.5}
    matrix arrow_y f 6 = {0.0, 0.0, 0.2, 0.0, -0.2, 0.0}
    matrix arrow2_x f 6 = {-0.5, 0.3, 0.3, 0.5, 0.3, 0.3}
    matrix arrow2_y f 6 = {0.0, 0.0,   0.2, 0.0, -0.2, 0.0}

    circulation $w

    set narr 6
    set fill 0

    $w cmd plsvect arrow_x arrow_y $narr $fill
    constriction $w

    set fill 1
    $w cmd plsvect arrow2_x  arrow2_y $narr $fill
    constriction $w

    potential $w
}

# Vector plot of circulation about the origin
proc circulation {w} {
    set nx 20
    set ny 20

    set dx 1.0
    set dy 1.0

    set xmin [expr (-$nx/2*$dx)]
    set xmax [expr ($nx/2*$dx)]
    set ymin [expr (-$ny/2*$dy)]
    set ymax [expr ($ny/2*$dy)]

    matrix xg f $nx $ny
    matrix yg f $nx $ny
    matrix u f $nx $ny
    matrix v f $nx $ny

    # Create data - circulation around the origin.
    for {set i 0} {$i < $nx} {incr i} {
	set x [expr (($i-$nx/2+0.5)*$dx) ]
	for {set j 0} {$j < $ny} {incr j} {
	    set y [expr (($j-$ny/2+0.5)*$dy)]
	    xg $i $j = $x
	    yg $i $j = $y
	    u $i $j = $y
	    v $i $j = [expr (-1.0*$x)]
	}
    }

    # Plot vectors with default arrows
    $w cmd plenv $xmin $xmax $ymin $ymax 0 0
    $w cmd pllab "(x)" "(y)" "#frPLplot Example 22 - circulation"
    $w cmd plcol0 2
    $w cmd plvect u v 0.0 "pltr2" xg yg
    $w cmd plcol0 1

}

# Vector plot of flow through a constricted pipe
proc constriction {w} {

    set pi 3.14159265358979323846

    set nx 20
    set ny 20

    set dx 1.0
    set dy 1.0

    set xmin [expr (-$nx/2*$dx)]
    set xmax [expr ($nx/2*$dx)]
    set ymin [expr (-$ny/2*$dy)]
    set ymax [expr ($ny/2*$dy)]

    matrix xg f $nx $ny
    matrix yg f $nx $ny
    matrix u f $nx $ny
    matrix v f $nx $ny

    set Q 2.0
    # Create data - circulation around the origin.
    for {set i 0} {$i < $nx} {incr i} {
	set x [expr (($i-$nx/2+0.5)*$dx) ]
	for {set j 0} {$j < $ny} {incr j} {
	    set y [expr (($j-$ny/2+0.5)*$dy)]
	    xg $i $j = $x
	    yg $i $j = $y
	    set b [expr ($ymax/4.0*(3.0-cos($pi*$x/$xmax)))]
	    if {abs($y) < $b} {
		set dbdx [expr ($ymax/4.0*sin($pi*$x/$xmax)*$y/$b)]
		u $i $j = [expr ($Q*$ymax/$b)]
		v $i $j = [expr ($Q*$ymax/$b*$dbdx)]
	    } else {
		u $i $j = 0.0
		v $i $j = 0.0
	    }
	}
    }

    # Plot vectors with default arrows
    $w cmd plenv $xmin $xmax $ymin $ymax 0 0
    $w cmd pllab "(x)" "(y)" "#frPLplot Example 22 - constriction"
    $w cmd plcol0 2
    $w cmd plvect u v -0.5 "pltr2" xg yg
    $w cmd plcol0 1

}

# Vector plot of the gradient of a shielded potential (see example 9)
proc potential {w} {

    set pi 3.14159265358979323846

    set nr 20
    set ntheta 20
    set nper 100
    set nlevel 10

    matrix xg f $nr $ntheta
    matrix yg f $nr $ntheta
    matrix u f $nr $ntheta
    matrix v f $nr $ntheta
    matrix z f $nr $ntheta

    # Potential inside a conducting cylinder (or sphere) by method of images.
    # Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
    # Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
    # Also put in smoothing term at small distances.

    set rmax $nr

    set eps [expr 2.]

    set q1 [expr 1.]
    set d1 [expr $rmax/4.]

    set q1i [expr - $q1*$rmax/$d1]
    set d1i [expr pow($rmax,2)/$d1]

    set q2 [expr -1.]
    set d2 [expr $rmax/4.]

    set q2i [expr - $q2*$rmax/$d2]
    set d2i [expr pow($rmax,2)/$d2]
    
    for {set i 0} {$i < $nr} {incr i} {
        set r [expr 0.5 + $i]
        for {set j 0} {$j < $ntheta} {incr j} {
            set theta [expr {(2. * $pi  / ($ntheta - 1.))*(0.5 + $j)}]
	    set x [expr {$r * cos($theta)}]
	    set y [expr {$r * sin($theta)}]
            xg $i $j = $x
            yg $i $j = $y
	    set div1 [expr {sqrt(pow($x-$d1,2) + pow($y-$d1,2) +
				 pow($eps,2))}]
	    set div1i [expr {sqrt(pow($x-$d1i,2) + pow($y-$d1i,2) + pow($eps,2))}]
	    set div2 [expr {sqrt(pow($x-$d2,2) + pow($y+$d2,2) +
				 pow($eps,2))}]
	    set div2i [expr {sqrt(pow($x-$d2i,2) + pow($y+$d2i,2) + pow($eps,2))}]
	    z $i $j = [expr {$q1/$div1 + $q1i/$div1i + $q2/$div2 + $q2i/$div2i}]
	    u $i $j = [expr {-$q1*($x-$d1)/pow($div1,3) - $q1i*($x-$d1i)/pow($div1i,3) - 
			     $q2*($x-$d2)/pow($div2,3) - $q2i*($x-$d2i)/pow($div2i,3)}]
	    v $i $j = [expr {-$q1*($y-$d1)/pow($div1,3) - $q1i*($y-$d1i)/pow($div1i,3) - 
			     $q2*($y+$d2)/pow($div2,3) - $q2i*($y+$d2i)/pow($div2i,3)}]
        }
    }

    set xmin [xg 0 0]
    set xmax $xmin
    set ymin [yg 0 0]
    set ymax $ymin
    set zmin [z 0 0]
    set zmax $zmin
    for {set i 0} {$i < $nr} {incr i} {
       for {set j 0} {$j < $ntheta} {incr j} {
          if {[xg $i $j] < $xmin} { set xmin [xg $i $j] }
          if {[xg $i $j] > $xmax} { set xmax [xg $i $j] }
          if {[yg $i $j] < $ymin} { set ymin [yg $i $j] }
          if {[yg $i $j] > $ymax} { set ymax [yg $i $j] }
          if {[z $i $j] < $zmin} { set zmin [z $i $j] }
          if {[z $i $j] > $zmax} { set zmax [z $i $j] }
       }
    }


    $w cmd plenv $xmin $xmax $ymin $ymax 0 0
    $w cmd pllab "(x)" "(y)" "#frPLplot Example 22 - potential gradient vector plot"
    # Plot contours of the potential
    set dz [expr (($zmax-$zmin)/$nlevel)]
    matrix clevel f $nlevel
    for {set i 0} {$i < $nlevel} {incr i} {
       clevel $i = [expr {$zmin + ($i + 0.5)*$dz}]
     }
    $w cmd plcol0 3
    $w cmd pllsty 2
    $w cmd plcont z clevel "pltr2" xg yg
    $w cmd pllsty 1
    $w cmd plcol0 1

    # Plot vectors with default arrows
    $w cmd plcol0 2
    $w cmd plvect u v 25.0 "pltr2" xg yg
    $w cmd plcol0 1

    # Plot the perimeter of the cylinder
    matrix px f $nper
    matrix py f $nper
    set dtheta [expr (2.0*$pi/($nper-1.0))]
    for {set i 0} {$i < $nper} {incr i} {
	set theta [expr $dtheta*$i]
	px $i = [expr ($rmax*cos($theta))]
	py $i = [expr ($rmax*sin($theta))]
    }
    $w cmd plline $nper px py

}

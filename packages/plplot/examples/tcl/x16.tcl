#----------------------------------------------------------------------------
# $Id: x16.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

# plshades demo, using color fill.

proc x16 {{w loopback}} {

    set ns 20
    set nx 35
    set ny 46

    set pi 3.14159265358979323846
    set fill_width 2; set cont_color 0; set cont_width 0

    matrix clevel f $ns
    matrix shedge f [expr $ns+1]
    matrix xg1 f $nx
    matrix yg1 f $ny
    matrix xg2 f $nx $ny
    matrix yg2 f $nx $ny
    matrix zz f $nx $ny
    matrix ww f $nx $ny

# Set up data array

    for {set i 0} {$i < $nx} {incr i} {
	set x [expr double($i - ($nx/2)) / double($nx/2)]
	for {set j 0} {$j < $ny} {incr j} {
	    set y [expr double($j - $ny/2) / double($ny/2) - 1.]

	    zz $i $j = [expr -sin(7.*$x) * cos(7.*$y) + $x*$x - $y*$y ]
	    ww $i $j = [expr -cos(7.*$x) * sin(7.*$y) + 2 * $x * $y ]
	}
    }

    set zmin [zz 0 0]
    set zmax $zmin
    for {set i 0} {$i < $nx} {incr i} {
	for {set j 0} {$j < $ny} {incr j} {
	    if {[zz $i $j] < $zmin} { set zmin [zz $i $j] }
	    if {[zz $i $j] > $zmax} { set zmax [zz $i $j] }
	}
    }

    for {set i 0} {$i < $ns} {incr i} {
	clevel $i = [expr $zmin + ($zmax - $zmin) * ($i + .5) / $ns.]
    }

    for {set i 0} {$i < [expr $ns+1]} {incr i} {
	shedge $i = [expr $zmin + ($zmax - $zmin) * double($i) / double($ns)]
    }

# Build the 1-d coord arrays.

    set distort .4

    for {set i 0} {$i < $nx} {incr i} {
	set xx [expr -1. + $i * ( 2. / ($nx-1.) )]
	xg1 $i = [expr $xx + $distort * cos( .5 * $pi * $xx ) ]
    }

    for {set j 0} {$j < $ny} {incr j} {
	set yy [expr -1. + $j * ( 2. / ($ny-1.) )]
	yg1 $j = [expr $yy - $distort * cos( .5 * $pi * $yy ) ]
    }

# Build the 2-d coord arrays.

    for {set i 0} {$i < $nx} {incr i} {
	set xx [expr -1. + $i * ( 2. / ($nx-1.) )]
	for {set j 0} {$j < $ny} {incr j} {
	    set yy [expr -1. + $j * ( 2. / ($ny-1.) )]

	    set argx [expr .5 * $pi * $xx]
	    set argy [expr .5 * $pi * $yy]

	    xg2 $i $j = [expr $xx + $distort * cos($argx) * cos($argy) ]
	    yg2 $i $j = [expr $yy - $distort * cos($argx) * cos($argy) ]
	}
    }

# Plot using identity transform

    $w cmd pladv 0
    $w cmd plvpor 0.1 0.9 0.1 0.9
    $w cmd plwind -1.0 1.0 -1.0 1.0

    $w cmd plpsty 0

    $w cmd plshades zz -1. 1. -1. 1. \
      shedge $fill_width $cont_color $cont_width \
      1

    $w cmd plcol0 1
    $w cmd plbox "bcnst" 0.0 0 "bcnstv" 0.0 0
    $w cmd plcol0 2

#    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, mypltr, NULL);

    $w cmd pllab "distance" "altitude" "Bogon density"

# Plot using 1d coordinate transform
    
    $w cmd pladv 0
    $w cmd plvpor 0.1 0.9 0.1 0.9
    $w cmd plwind -1.0 1.0 -1.0 1.0

    $w cmd plpsty 0

    $w cmd plshades zz -1. 1. -1. 1. \
      shedge $fill_width $cont_color $cont_width \
      1 pltr1 xg1 yg1

    $w cmd plcol0 1
    $w cmd plbox "bcnst" 0.0 0 "bcnstv" 0.0 0
    $w cmd plcol0 2

#    plcont(w, nx, ny, 1, nx, 1, ny, clevel, ns, pltr1, (void *) &cgrid1);

    $w cmd pllab "distance" "altitude" "Bogon density"

# Plot using 2d coordinate transform

    $w cmd pladv 0
    $w cmd plvpor 0.1 0.9 0.1 0.9
    $w cmd plwind -1.0 1.0 -1.0 1.0

    $w cmd plpsty 0

    $w cmd plshades zz -1. 1. -1. 1. \
      shedge $fill_width $cont_color $cont_width \
      0 pltr2 xg2 yg2

    $w cmd plcol0 1
    $w cmd plbox "bcnst" 0.0 0 "bcnstv" 0.0 0
    $w cmd plcol0 2
    $w cmd plcont ww clevel pltr2 xg2 yg2

    $w cmd pllab "distance" "altitude" "Bogon density, with streamlines"

# Plot using 2d coordinate transform with both shades and contours.

    $w cmd pladv 0
    $w cmd plvpor 0.1 0.9 0.1 0.9
    $w cmd plwind -1.0 1.0 -1.0 1.0

    $w cmd plpsty 0

    $w cmd plshades zz -1. 1. -1. 1. \
      shedge $fill_width 2 3 \
      0 pltr2 xg2 yg2

    $w cmd plcol0 1
    $w cmd plbox "bcnst" 0.0 0 "bcnstv" 0.0 0
    $w cmd plcol0 2

    $w cmd pllab "distance" "altitude" "Bogon density"

# Polar plot example demonstrating wrapping support.

# Build the new coordinate matrices.

    set nylim [expr $ny - 1]; set wrap 2;
    matrix xg f $nx $nylim
    matrix yg f $nx $nylim
    matrix z  f $nx $nylim

    for {set i 0} {$i < $nx} {incr i} {
	set r [expr $i / ($nx - 1.)]
	for {set j 0} {$j < $nylim} {incr j} {
	    set t [expr 2. * $pi * $j / ($ny - 1.)]

	    xg $i $j = [expr $r * cos($t)]
	    yg $i $j = [expr $r * sin($t)]

	    z $i $j = [expr exp(-$r*$r) * cos(5.*$t) * cos(5.*$pi*$r) ]
	}
    }

# Need a new shedge to go along with the new data set.

    set zmin [z 0 0]
    set zmax $zmin
    for {set i 0} {$i < $nx} {incr i} {
	for {set j 0} {$j < $nylim} {incr j} {
	    if {[z $i $j] < $zmin} { set zmin [z $i $j] }
	    if {[z $i $j] > $zmax} { set zmax [z $i $j] }
	}
    }

    for {set i 0} {$i < [expr $ns+1]} {incr i} {
	shedge $i = [expr $zmin + ($zmax - $zmin)/double($ns) * double($i)]
    }

    $w cmd pladv 0
    $w cmd plvpor 0.1 0.9 0.1 0.9
    $w cmd plwind -1.0 1.0 -1.0 1.0

    $w cmd plpsty 0

    $w cmd plshades z -1. 1. -1. 1. \
      shedge $fill_width $cont_color $cont_width \
      0 pltr2 xg yg $wrap

# Hold perimeter
    matrix px f 100; matrix py f 100

    for {set i 0} {$i < 100} {incr i} {
	set t [expr 2. * $pi * $i / 99.]
	px $i = [expr cos($t)]
	py $i = [expr sin($t)]
    }
# draw the perimeter.
    $w cmd plcol0 1
    $w cmd plline 100 px py

# And label the plot.
    $w cmd plcol0 2
    $w cmd pllab "" "" "Tokamak Bogon Instability"
# Restore defaults
    $w cmd plcol0 1
}

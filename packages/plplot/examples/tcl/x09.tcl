#----------------------------------------------------------------------------
# $Id: x09.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

# Contour plot demo.

proc x09 {{w loopback}} {

    set xpts 35
    set ypts 46

    matrix clevel f 11 = {-1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1.}

    matrix mark  i 1 = { 1500 }
    matrix space i 1 = { 1500 }

    matrix zz f $xpts $ypts
    matrix ww f $xpts $ypts

# Calculate the data matrices.

    for {set i 0} {$i < $xpts} {incr i} {
	set xx [expr ($i - ($xpts / 2)) / double($xpts / 2) ]
	for {set j 0} {$j < $ypts} {incr j} {
	    set yy [expr ($j - ($ypts / 2)) / double($ypts / 2) - 1.0 ]
	    zz $i $j = [expr $xx * $xx - $yy * $yy ]
	    ww $i $j = [expr 2. * $xx * $yy ]
	}
    }

    matrix xg0 f $xpts
    matrix yg0 f $ypts

    matrix xg1 f $xpts
    matrix yg1 f $ypts

    matrix xg2 f $xpts $ypts
    matrix yg2 f $xpts $ypts

    set pi 3.14159265358979323846
    set distort .4

# Build the 1-d coord arrays.

    for {set i 0} {$i < $xpts} {incr i} {
	set xx [expr -1. + $i * ( 2. / ($xpts-1.) )]
	xg0 $i = [expr $xx]
	xg1 $i = [expr $xx + $distort * cos( .5 * $pi * $xx ) ]
    }

    for {set j 0} {$j < $ypts} {incr j} {
	set yy [expr -1. + $j * ( 2. / ($ypts-1.) )]
	yg0 $j = [expr $yy]
	yg1 $j = [expr $yy - $distort * cos( .5 * $pi * $yy ) ]
    }

# Build the 2-d coord arrays.

    for {set i 0} {$i < $xpts} {incr i} {
	set xx [expr -1. + $i * ( 2. / ($xpts-1.) )]
	for {set j 0} {$j < $ypts} {incr j} {
	    set yy [expr -1. + $j * ( 2. / ($ypts-1.) )]

	    set argx [expr .5 * $pi * $xx]
	    set argy [expr .5 * $pi * $yy]

	    xg2 $i $j = [expr $xx + $distort * cos($argx) * cos($argy) ]
	    yg2 $i $j = [expr $yy - $distort * cos($argx) * cos($argy) ]
	}
    }

# Plot using scaled identity transformation used to create
# xg0 and yg0.  The implementation is different, but this gives
# the same results as the mypltr transformation for the first
# plots in x09c.

#    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 0
#    $w cmd plenv -1.0 1.0 -1.0 1.0 0 0
#    $w cmd plcol0 2
#    $w cmd plcont zz clevel pltr1 xg0 yg0
#    $w cmd plstyl 1 mark space
#    $w cmd plcol0 3
#    $w cmd plcont ww clevel pltr1 xg0 yg0
#    $w cmd plstyl 0 mark space
#    $w cmd plcol0 1
#    $w cmd pllab "X Coordinate" "Y Coordinate" "Streamlines of flow"

    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 1
    $w cmd plenv -1.0 1.0 -1.0 1.0 0 0
    $w cmd plcol0 2
    $w cmd plcont zz clevel pltr1 xg0 yg0
    $w cmd plstyl 1 mark space
    $w cmd plcol0 3
    $w cmd plcont ww clevel pltr1 xg0 yg0
    $w cmd plstyl 0 mark space
    $w cmd plcol0 1
    $w cmd pllab "X Coordinate" "Y Coordinate" "Streamlines of flow"

# Plot using 1d coordinate transform

    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 0
    $w cmd plenv -1.0 1.0 -1.0 1.0 0 0
    $w cmd plcol0 2
    $w cmd plcont zz clevel pltr1 xg1 yg1
    $w cmd plstyl 1 mark space
    $w cmd plcol0 3
    $w cmd plcont ww clevel pltr1 xg1 yg1
    $w cmd plstyl 0 mark space
    $w cmd plcol0 1
    $w cmd pllab "X Coordinate" "Y Coordinate" "Streamlines of flow"

#    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 1
#    $w cmd plenv -1.0 1.0 -1.0 1.0 0 0
#    $w cmd plcol0 2
#    $w cmd plcont zz clevel pltr1 xg1 yg1
#    $w cmd plstyl 1 mark space
#    $w cmd plcol0 3
#    $w cmd plcont ww clevel pltr1 xg1 yg1
#    $w cmd plstyl 0 mark space
#    $w cmd plcol0 1
#    $w cmd pllab "X Coordinate" "Y Coordinate" "Streamlines of flow"

# Plot using 2d coordinate transform

#    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 0
    $w cmd plenv -1.0 1.0 -1.0 1.0 0 0
    $w cmd plcol0 2
    $w cmd plcont zz clevel pltr2 xg2 yg2
    $w cmd plstyl 1 mark space
    $w cmd plcol0 3
    $w cmd plcont ww clevel pltr2 xg2 yg2
    $w cmd plstyl 0 mark space
    $w cmd plcol0 1
    $w cmd pllab "X Coordinate" "Y Coordinate" "Streamlines of flow"

#    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 1
#    $w cmd plenv -1.0 1.0 -1.0 1.0 0 0
#    $w cmd plcol0 2
#    $w cmd plcont zz clevel pltr2 xg2 yg2
#    $w cmd plstyl 1 mark space
#    $w cmd plcol0 3
#    $w cmd plcont ww clevel pltr2 xg2 yg2
#    $w cmd plstyl 0 mark space
#    $w cmd plcol0 1
#    $w cmd pllab "X Coordinate" "Y Coordinate" "Streamlines of flow"

    #polar contour example.
    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 0
    x09_polar $w

#    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 1
#    x09_polar $w

    #potential contour example.
    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 0
    x09_potential $w

#    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 1
#    x09_potential $w

# Restore defaults
    $w cmd plcol0 1
#    $w cmd pl_setcontlabelparam 0.006 0.3 0.1 0
}

# Demonstrate plotting of wrapped data.  What is significant to
# understand about this example is that for the common case of
# plotting polar data (or other forms of coordinates that wrap on
# themselves) you can do it from Tcl /without/ having to go to the
# trouble to construct a special data plotting matrix with an extra
# row or column and then copy the data into it, replicating the first
# row/col into the extra row/col.

proc x09_polar {{w loopback}} {

    set pi 3.14159265358979323846
    $w cmd plenv -1 1 -1 1 0 -2
    $w cmd plcol0 1

# Hold perimeter
    matrix px f 100; matrix py f 100

    for {set i 0} {$i < 100} {incr i} {
	set t [expr 2. * $pi * $i / 99.]
	px $i = [expr cos($t)]
	py $i = [expr sin($t)]
    }

    $w cmd plline 100 px py

    set xpts 40; set ypts 40; set ylim [expr $ypts - 1]; set wrap 2
    matrix xg f $xpts $ylim
    matrix yg f $xpts $ylim
    matrix z f $xpts $ylim

    for {set i 0} {$i < $xpts} {incr i} {
	set r [expr $i / ($xpts - 1.)]
	for {set j 0} {$j < $ylim} {incr j} {
	    set t [expr 2. * $pi * $j / ($ypts - 1.)]

	    xg $i $j = [expr $r * cos($t)]
	    yg $i $j = [expr $r * sin($t)]

	    z $i $j = $r
	}
    }

    matrix lev f 10 = { .05, .15, .25, .35, .45, .55, .65, .75, .85, .95 }

    $w cmd plcol0 2
    $w cmd plcont z lev pltr2 xg yg $wrap

    $w cmd plcol0 1
    $w cmd pllab "" "" "Polar Contour Plot"
}

proc x09_potential {{w loopback}} {
# Shielded potential contour plot example

    set pi 3.14159265358979323846
    set xpts 40; set ypts 64; set ylim [expr $ypts - 1]; set wrap 2;
    set perimeterpts 100; set nlevel 20

    # Create data to be contoured.
    matrix xg f $xpts $ylim
    matrix yg f $xpts $ylim
    matrix z f $xpts $ylim

    for {set i 0} {$i < $xpts} {incr i} {
	set r [expr 0.5 + $i]
	for {set j 0} {$j < $ylim} {incr j} {
	    set theta [expr {(2. * $pi  / ($ypts - 1.))*(0.5 + $j)}]

	    xg $i $j = [expr {$r * cos($theta)}]
	    yg $i $j = [expr {$r * sin($theta)}]
	}
    }

    set rmax $r
    set xmin [xg 0 0]
    set xmax $xmin
    set ymin [yg 0 0]
    set ymax $ymin
    for {set i 0} {$i < $xpts} {incr i} {
       for {set j 0} {$j < $ylim} {incr j} {
	  if {[xg $i $j] < $xmin} { set xmin [xg $i $j] }
	  if {[xg $i $j] > $xmax} { set xmax [xg $i $j] }
	  if {[yg $i $j] < $ymin} { set ymin [yg $i $j] }
	  if {[yg $i $j] > $ymax} { set ymax [yg $i $j] }
       }
    }
    set x0 [expr ($xmin + $xmax)/2.]
    set y0 [expr ($ymin + $ymax)/2.]

    # Expanded limits.

    set peps [expr 0.05]
    set xpmin [expr $xmin - abs($xmin)*$peps]
    set xpmax [expr $xmax + abs($xmax)*$peps]
    set ypmin [expr $ymin - abs($ymin)*$peps]
    set ypmax [expr $ymax + abs($ymax)*$peps]

    # Potential inside a conducting cylinder (or sphere) by method of images.
    # Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
    # Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
    # Also put in smoothing term at small distances.

    set eps [expr 2.]

    set q1 [expr 1.]
    set d1 [expr $rmax/4.]

    set q1i [expr - $q1*$rmax/$d1]
    set d1i [expr pow($rmax,2)/$d1]

    set q2 [expr -1.]
    set d2 [expr $rmax/4.]

    set q2i [expr - $q2*$rmax/$d2]
    set d2i [expr pow($rmax,2)/$d2]

    for {set i 0} {$i < $xpts} {incr i} {
	for {set j 0} {$j < $ylim} {incr j} {
  	   set div1 [expr {sqrt(pow([xg $i $j]-$d1,2) + pow([yg $i $j]-$d1,2) + pow($eps,2))}]
  	   set div1i [expr {sqrt(pow([xg $i $j]-$d1i,2) + pow([yg $i $j]-$d1i,2) + pow($eps,2))}]
  	   set div2 [expr {sqrt(pow([xg $i $j]-$d2,2) + pow([yg $i $j]+$d2,2) + pow($eps,2))}]
  	   set div2i [expr {sqrt(pow([xg $i $j]-$d2i,2) + pow([yg $i $j]+$d2i,2) + pow($eps,2))}]
	   z $i $j = [expr {$q1/$div1 + $q1i/$div1i + $q2/$div2 + $q2i/$div2i}]
	}
     }

    set zmin [z 0 0]
    set zmax $zmin
    for {set i 0} {$i < $xpts} {incr i} {
	for {set j 0} {$j < $ylim} {incr j} {
	    if {[z $i $j] < $zmin} { set zmin [z $i $j] }
	    if {[z $i $j] > $zmax} { set zmax [z $i $j] }
	}
    }

    # Positive and negative contour levels.
    set dz [expr ($zmax-$zmin)/$nlevel]
    set nlevelneg [expr 0]
    set nlevelpos [expr 0]
    matrix clevelneg f $nlevel
    matrix clevelpos f $nlevel
    for {set i 0} {$i < $nlevel} {incr i} {
       set clevel [expr {$zmin + ($i + 0.5)*$dz}]
       if {$clevel <= 0.} {
     	  clevelneg $nlevelneg = $clevel; incr nlevelneg
       } else {
	  clevelpos $nlevelpos = $clevel; incr nlevelpos
       }
     }

     # Colours!
     set ncollin [expr 11]
     set ncolbox [expr 1]
     set ncollab [expr 2]
     
     # Finally start plotting this page!
     $w cmd pladv 0
     $w cmd plcol0 $ncolbox

     $w cmd plvpas 0.1 0.9 0.1 0.9 1.0
     $w cmd plwind $xpmin $xpmax $ypmin $ypmax
     $w cmd plbox "" 0. 0 "" 0. 0

     $w cmd plcol0 $ncollin
     if {$nlevelneg >0} {
	# Negative contours
	# copy partially full clevelneg to full levneg required by plcont
	matrix levneg f $nlevelneg
    	for {set i 0} {$i < $nlevelneg} {incr i} {
	   levneg $i = [clevelneg $i]
	}
	$w cmd pllsty 2
    	$w cmd plcont z levneg pltr2 xg yg $wrap
     }
     
     if {$nlevelpos >0} {
	# Positive contours
	# copy partially full clevelpos to full levpos required by plcont
	matrix levpos f $nlevelpos
    	for {set i 0} {$i < $nlevelpos} {incr i} {
	   levpos $i = [clevelpos $i]
	}
	$w cmd pllsty 1
    	$w cmd plcont z levpos pltr2 xg yg $wrap
     }
     
    #Draw outer boundary
    matrix px f $perimeterpts
    matrix py f $perimeterpts
    for {set i 0} {$i < $perimeterpts} {incr i} {
       set t [expr {(2.*$pi/($perimeterpts-1))*$i}]
	px $i = [expr {$x0 + $rmax*cos($t)}]
	py $i = [expr {$y0 + $rmax*sin($t)}]
    }

    $w cmd plcol0 $ncolbox
    $w cmd plline $perimeterpts px py

    $w cmd plcol0 $ncollab
    $w cmd pllab "" "" "Shielded potential of charges in a conducting sphere"
}

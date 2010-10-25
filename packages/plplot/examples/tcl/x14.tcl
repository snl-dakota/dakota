#----------------------------------------------------------------------------
# $Id: x14.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

proc x14 {{w loopback}} {
    global xscale yscale xoff yoff
    set geometry_master "500x410+100+200"
    set geometry_slave  "500x410+650+200"

    $w cmd plgdev driver
    puts "Demo of multiple output streams via the $driver driver."
    puts "Running with the second stream as slave to the first."
    puts ""
    
# Set up first stream
    $w cmd plsetopt "geometry" $geometry_master
    
    if {$driver != ""} {$w cmd plsdev $driver}
    $w cmd plssub 2 2
    $w cmd plinit
	
# Start next stream
	
    $w cmd plsstrm 1
    
# Turn off pause to make this a slave (must follow master)
    
    $w cmd plsetopt "geometry" $geometry_slave
    $w cmd plspause 0
    if {$driver != ""} {$w cmd plsdev $driver}
    $w cmd plinit
    
# Set up the data & plot
# Original case
    
    $w cmd plsstrm 0

    set xscale 6.0
    set yscale 1.0
    set xoff 0.0
    set yoff 0.0

    plot141 $w

# Set up the data & plot

    set xscale 1.
    set yscale 1.e+6
    plot141 $w
	    
# Set up the data & plot
    
    set xscale 1.
    set yscale 1.e-6
    set digmax 2
    $w cmd plsyax digmax 0
    plot141 $w
    
# Set up the data & plot
    
    set xscale 1.
    set yscale 0.0014
    set yoff 0.0185
    set digmax 5
    $w cmd plsyax digmax 0
    plot141 $w
    
# To slave
# The pleop() ensures the eop indicator gets lit.

    $w cmd plsstrm 1
    plot144 $w
    $w cmd pleop 
    
# Back to master
    
    $w cmd plsstrm 0
    plot142 $w
    plot143 $w
							
# To slave */

    $w cmd plsstrm 1
    plot145 $w
    
    $w cmd pleop 
    
# Back to master to wait for user to advance
    
    $w cmd plsstrm 0
    $w cmd pleop
		    
# Call plend to finish off.
		    
    $w cmd plend
}

# This is supposed to work just like the plot1() in x01c.c

proc plot141 {w} {
    global xscale yscale xoff yoff

    set npts 60
    matrix x f $npts
    matrix y f $npts

    for {set i 0} {$i < $npts} {incr i} {
	x $i = [expr $xoff + ($xscale * ($i + 1)) / $npts]
	y $i = [expr $yoff + $yscale * pow([x $i],2)]
    }

    set xmin [x [expr 0]]
    set xmax [x [expr $npts-1]]
    set ymin [y [expr 0]]
    set ymax [y [expr $npts-1]]

    matrix x1 f 6
    matrix y1 f 6

    for {set i 0} {$i < 6} {incr i} {
	set j [expr $i*10+3]
	x1 $i = [x $j]
	y1 $i = [y $j]
    }

    $w cmd plcol0 1
    $w cmd plenv $xmin $xmax $ymin $ymax 0 0
    $w cmd plcol0 6
    $w cmd pllab "(x)" "(y)" "#frPLplot Example 1 - y=x#u2"

    # plot the data points

    $w cmd plcol0 9
    $w cmd plpoin 6 x1 y1 9

    # draw the line through the data

    $w cmd plcol0 4
    $w cmd plline $npts x y
    $w cmd plflush
}

# This is supposed to work just like the plot2() in x01c

proc plot142 {w} {
    $w cmd plcol0 1
    $w cmd plenv -2 10 -.4 1.2 0 1
    $w cmd plcol0 2
    $w cmd pllab "(x)" "sin(x)/x" "#frPLplot Example 1 - Sinc Function"

# Fill up the array

    matrix x1 f 101
    matrix y1 f 101

    for {set i 0} {$i < 100} {incr i} {
	set x [expr ($i - 19.)/6.]
	x1 $i = $x
	y1 $i = 1
	if {$x != 0} { y1 $i = [expr sin($x)/$x] }
    }

    $w cmd plcol0 3
    $w cmd plline 100 x1 y1
    $w cmd plflush
}

# This is supposed to work just like the plot3() in x01c.c

proc plot143 {w} {

    set pi 3.14159265358979323846
    $w cmd pladv 0
    $w cmd plvsta
    $w cmd plwind 0.0 360.0 -1.2 1.2

# Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.

    $w cmd plcol0 1
    $w cmd plbox "bcnst" 60.0 2 "bcnstv" 0.2 2

# Superimpose a dashed line grid, with 1.5 mm marks and spaces. 
# plstyl expects two integer matrices for mark and space!

    matrix mark i 1
    matrix space i 1

    mark 0 = 1500
    space 0 = 1500
    $w cmd plstyl 1 mark space

    $w cmd plcol0 2
    $w cmd plbox "g" 30.0 0 "g" 0.2 0

    mark 0 = 0
    space 0 = 0
    $w cmd plstyl 0 mark space

    $w cmd plcol0 3
    $w cmd pllab "Angle (degrees)" "sine" "#frPLplot Example 1 - Sine function"

    matrix x f 101
    matrix y f 101

    for {set i 0} {$i < 101} {incr i} {
	x $i = [expr 3.6 * $i]
	y $i = [expr sin([x $i] * $pi / 180.0)]
    }

    $w cmd plcol0 4
    $w cmd plline 101 x y
    $w cmd plflush
}

# This is supposed to work like example 3.
proc plot144 {w} {
    set twopi  [expr 2. * 3.14159265358979323846]
# Set up viewport and window, but do not draw box 

    $w cmd plenv -1.3 1.3 -1.3 1.3 1 -2

# Draw circles for polar grid

    set ni 10
    set nj 360
    set nj1 [expr $nj + 1]

    set dr     [expr 1. / $ni]
    set dtheta [expr $twopi / $nj]

    matrix xj f $nj1
    matrix yj f $nj1

    for {set i 1} {$i <= $ni} {incr i} {
	for {set j 0} {$j < $nj1} {incr j} {
	    set r     [expr $i * $dr]
	    set theta [expr $j * $dtheta]
	    xj $j = [expr $r * cos($theta)]
	    yj $j = [expr $r * sin($theta)]
	}
	$w cmd plline $nj1 xj yj
    }

# Draw radial spokes for polar grid and write labels for angle

    $w cmd plcol0 2
    for {set j 0} {$j <= 11} {incr j} {
	set theta [expr $j * $twopi / 12.]
	set xg [expr cos($theta)]
	set yg [expr sin($theta)]
	$w cmd pljoin 0.0 0.0 $xg $yg

# Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
	if {$xg >= -0.00001} {
	    set dx $xg
	    set dy $yg
	    set just -0.15
	} else {
	    set dx [expr -$xg]
	    set dy [expr -$yg]
	    set just 1.15
	}
	set label [expr round($theta*360./$twopi)]

# N.B. cannot get this command to give same postscript output.  Also visual 
# inspection shows 90 deg label jumping around slightly compared to python
# and C front ends.  No idea why (AWI comment).
	$w cmd plptex $xg $yg $dx $dy $just $label
    }

# Draw the graph 

    set npts 360
    set npts1 [expr $npts+1]

    set dtheta [expr $twopi / $npts]

    matrix x f $npts1
    matrix y f $npts1

    for {set j 0} {$j <= $npts} {incr j} {
	set theta [expr $j * $dtheta]
	set r     [expr sin(5 * $theta)]
	x $j = [expr $r * cos($theta)]
	y $j = [expr $r * sin($theta)]
    }
    $w cmd plcol0 3
    $w cmd plline $npts1 x y

    $w cmd plcol0 4
    $w cmd plmtex "t" 2.0 0.5 0.5 "#frPLplot Example 3 - r(#gh)=sin 5#gh"
    $w cmd plflush
}

# This is supposed to work like first page of example 9.
proc plot145 {w} {
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

# Build the 1-d coord arrays.

    for {set i 0} {$i < $xpts} {incr i} {
	set xx [expr -1. + $i * ( 2. / ($xpts-1.) )]
	xg0 $i = [expr $xx]
    }

    for {set j 0} {$j < $ypts} {incr j} {
	set yy [expr -1. + $j * ( 2. / ($ypts-1.) )]
	yg0 $j = [expr $yy]
    }

# Plot using scaled identity transformation used to create
# xg0 and yg0.  The implementation is different, but this gives
# the same results as the mypltr transformation for the first
# plots in x09c.

    $w cmd plenv -1.0 1.0 -1.0 1.0 0 0
    $w cmd plcol0 2
    $w cmd plcont zz clevel pltr1 xg0 yg0
    $w cmd plstyl 1 mark space
    $w cmd plcol0 3
    $w cmd plcont ww clevel pltr1 xg0 yg0
    $w cmd plstyl 0 mark space
    $w cmd plcol0 1
    $w cmd pllab "X Coordinate" "Y Coordinate" "Streamlines of flow"
    $w cmd plflush
}

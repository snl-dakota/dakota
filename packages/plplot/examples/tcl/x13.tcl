#----------------------------------------------------------------------------
# $Id: x13.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

# Does a simple pie chart.

proc x13 {{w loopback}} {

    set text0 "Maurice"
    set text1 "Geoffrey"
    set text2 "Alan"
    set text3 "Rafael"
    set text4 "Vince"

#    int i, j;
#    PLFLT dthet, theta0, theta1, theta, just, dx, dy;
#    static PLFLT x[500], y[500], per[5];

    matrix x f 500
    matrix y f 500
    matrix per f 5 = {10., 32., 12., 30., 16.}

    $w cmd plenv 0. 10. 0. 10. 1 -2
    $w cmd plcol0 2

    set theta0 0.;
    set pi 3.14159265358979323846

    set dthet [expr 2. * $pi / 500.]
    for {set i 0} {$i <= 4} {incr i} {
	set j 0;
	x $j = 5.;
	y $j = 5.;
	incr j

	set theta1 [expr $theta0 + 2. * $pi * [per $i] / 100.]
	if {$i == 4} {
	    set theta1 [expr 2. * $pi]
	}

	for {set theta $theta0} {$theta <= $theta1} {
	    set theta [expr $theta + $dthet]} {
	    x $j = [expr 5. + 3. * cos($theta) ]
	    y $j = [expr 5. + 3. * sin($theta) ]
	    incr j
	}

	$w cmd plcol0 [expr $i + 1]
	$w cmd plpsty [expr (($i + 3) % 8 + 1)]
	$w cmd plfill $j x y
	$w cmd plcol0 1
	$w cmd plline $j x y
	set just [expr ($theta0 + $theta1) / 2.]
	set dx [expr .25 * cos($just)]
	set dy [expr .25 * sin($just)]
	if {$just < $pi / 2. || $just > 3. * $pi / 2.} {
	    set just 0.
	} else {
	    set just 1.
	}

	$w cmd plptex [expr [x [expr $j / 2]] + $dx] \
	    [expr [y [expr $j / 2]] + $dy] 1.0 0.0 $just [set text$i]
	set theta0 [expr $theta - $dthet]
    }
    $w cmd plfont 2
    $w cmd plschr 0. 1.3
    $w cmd plptex 5.0 9.0 1.0 0.0 0.5 "Percentage of Sales"
# Restore defaults
    $w cmd plcol0 1
}

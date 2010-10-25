#----------------------------------------------------------------------------
# $Id: x12.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

# Does a simple bar chart, using color fill.  If color fill is
# unavailable, pattern fill is used instead (automatic).

proc x12 {{w loopback}} {

    matrix y0 f 10 = {5., 15., 12., 24., 28., 30., 20., 8., 12., 3.}

    $w cmd pladv 0
    $w cmd plvsta
    $w cmd plwind 1980.0 1990.0 0.0 35.0
    $w cmd plbox "bc" 1.0 0 "bcnv" 10.0 0
    $w cmd plcol0 2
    $w cmd pllab "Year" "Widget Sales (millions)" "#frPLplot Example 12"

    for {set i 0} {$i < 10} {incr i} {
	$w cmd plcol0 [expr $i+1]

#	$w cmd plcol1 [expr ($i + 1.)/10.0 ]

	$w cmd plpsty 0

	plfbox $w [expr 1980. + $i] [y0 $i]

	set string [format "%.0f" [y0 $i] ]
	$w cmd plptex [expr 1980. + $i + .5] [expr [y0 $i] + 1.] \
	    1.0 0.0 .5 $string

	set string [format "%d" [expr 1980 + $i] ]
	$w cmd plmtex "b" 1.0 [expr ($i + 1) * .1 - .05] 0.5 $string
    }
# Restore defaults
    $w cmd plcol0 1
}

proc plfbox  {w x0 y0} {

    matrix x f 4; matrix y f 4

    x 0 = $x0;
    y 0 = 0.;
    x 1 = $x0;
    y 1 = $y0;
    x 2 = [expr $x0 + 1.];
    y 2 = $y0;
    x 3 = [expr $x0 + 1.];
    y 3 = 0.;
    $w cmd plfill 4 x y
    $w cmd plcol0 1
    $w cmd pllsty 1
    $w cmd plline 4 x y
}

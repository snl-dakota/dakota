#----------------------------------------------------------------------------
# $Id: plgrid.tcl 3186 2006-02-15 18:17:33Z slbrow $
#
# Demo polar grid plotter
#
#
# This programme uses the loopback widget so it can be run from either pltcl 
# with a choice of the standard drivers or plserver with the plframe widget.
# (1) pltcl:
#     execute pltcl and type plinit to get a selection of
#     device types.  Choose one of the devices then type
#     "source plgrid.tcl" (this file), then type "plgrid" 
#     with no arguments.
# (2) plserver
#     execute plserver then type the following (see ../tk/tkdemos.tcl for
#     many other working examples of this method).
#     plstdwin .
#     plxframe .plw
#     pack append . .plw {left expand fill}
#     source plgrid.tcl
#     proc 1 {} "plgrid .plw.plwin"
#     to execute this procedure that you have just created execute the "1"
#     command.
#----------------------------------------------------------------------------

proc plgrid {{w loopback}} {
    set ni 10
    set nj 20
    set nj1 [expr $nj + 1]

    matrix xi f $ni
    matrix yi f $ni
    matrix xj f $nj1
    matrix yj f $nj1

    set dr     [expr 1. / $ni]
    set dtheta [expr 2. * 3.14159265358979323846 / $nj]

# Set up viewport and window, but do not draw box 

    $w cmd plssub 1 1
    $w cmd plcol 1
    $w cmd plenv -1.3 1.3 -1.3 1.3 1 -2

# Draw i-lines

    for {set i 0} {$i < $ni} {incr i} {
	for {set j 0} {$j < $nj1} {incr j} {
	    set r     [expr $i * $dr]
	    set theta [expr $j * $dtheta]
	    set psi   [expr $theta + 0.5 * $r * sin($theta)]

	    xj $j = [expr $r * cos($psi)]
	    yj $j = [expr $r * sin($psi)]
	}
	$w cmd plline $nj1 xj yj
    }

# Draw j-lines

    for {set j 0} {$j < $nj} {incr j} {
	for {set i 0} {$i < $ni} {incr i} {
	    set r     [expr $i * $dr]
	    set theta [expr $j * $dtheta]
	    set psi   [expr $theta + 0.5 * $r * sin($theta)]

	    xi $i = [expr $r * cos($psi)]
	    yi $i = [expr $r * sin($psi)]
	}
	$w cmd plline $ni xi yi
    }
}

#----------------------------------------------------------------------------
# $Id: tcldemos.tcl 3186 2006-02-15 18:17:33Z slbrow $
#
# PLplot TCL demos
# 
# Maurice LeBrun
# IFS, University of Texas at Austin
# 23-Jun-1994
#
# To plot these, start up pltcl and type plinit to get a selection of
# device types.  Choose one of the devices then type
# "source tcldemos.tcl", then type "1" for the first demo, "2" for the
# second, and so on.  Not all examples are fully implemented yet.
# Note if you use xwin for the device type, you must type CR in the
# xwin window to move from one plot to the next.
#
# Note: each demo proc is supplied a widget argument if run from a widget,
# or "loopback" if being run from pltcl.  In the latter case, the
# "loopback cmd" does nothing but is required to make the two styles of
# Tcl scripts compatible.
#----------------------------------------------------------------------------

for {set i 1} {$i <= 19} {incr i} {
    set demo x[format "%02d" $i]
    source $demo.tcl
    proc $i {} "$demo"
}
set i 22
set demo x[format "%02d" $i]
source $demo.tcl
proc $i {} "$demo"

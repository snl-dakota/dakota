#----------------------------------------------------------------------------
# $Id: tkdemos.tcl 3186 2006-02-15 18:17:33Z slbrow $
#
# PLplot TK demos
# 
# Maurice LeBrun
# IFS, University of Texas at Austin
# 26-Jan-1995
#
# To plot these, start up plserver.  Type "source tkdemos.tcl", at which
# point a plframe extended widget is created.  Then type "1" for the first
# demo, "2" for the second, and so on.
#
# Note: each demo proc is supplied a widget argument if run from a widget,
# or "loopback" if being run from pltcl.  In the latter case, the
# "loopback cmd" does nothing, but is required two make the two styles of
# Tcl scripts compatible.
#----------------------------------------------------------------------------

plstdwin .
plxframe .plw
pack append . .plw {left expand fill}

for {set i 1} {$i <= 19} {incr i} {
    set demo x[format "%02d" $i]
    source $demo.tcl
    proc $i {} "$demo .plw.plwin"
}

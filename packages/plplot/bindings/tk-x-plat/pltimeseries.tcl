## -*-Tcl-*-
 # ###################################################################
 # 
 #  FILE: "pltimeseries.tcl"
 #                                    created: 03/21/1998 {00:21:52 AM} 
 #                                last update: 07/01/2002 {06:11:57 PM} 
 #  Author: Vince Darley
 #  E-mail: vince@biosgroup.com
 #    mail: Bios Group
 #          317 Paseo de Peralta, Santa Fe, NM 87501
 #     www: http://www.biosgroup.com/
 #  
 # Copyright (c) 1998-1999 Vince Darley
 # 
 # All rights reserved.
 # ###################################################################
 ##

package require Plplotter
package require Itk 

proc pltimeseries {args} {uplevel Pltimeseries $args}

itcl::class Pltimeseries {
    inherit itk::Widget
    
    variable timespan 100
    variable min_time 0
    public variable max_time 100
    public variable scroll_every 16
    public variable y_min 9.0
    public variable y_max 21.0
    public variable actual_y_max -1000.0
    public variable auto_reduce_scale 1
    public variable auto_reduce_scale_minimum 0.75
    public variable auto_scale_freq 0
    public variable auto_scale_intelligently 0
    
    variable colour_increment 1.0
    
    itk_option define -title title Title "plot"
    itk_option define -yname yname Yname "y"
    itk_option define -xname xname Xname "time"

    private variable _tick 0
    private variable _ticks ""
    private variable _points 
    private variable _replot 0
    
    constructor {args} {}
    destructor {}
    
    method internal_set {var to} {
	set $var $to
    }
    method initialise {}
    method cmd {args} { uplevel 1 $itk_component(pl) cmd $args }
    method datapoints {args}
    method tick {} {}
    method plotpoint {t y}
    method plotline {t y}
    method plotaxes {}
    method test {}
}

itcl::body Pltimeseries::constructor {args} {
    # 
    # Create the outermost frame to maintain geometry.
    #
    itk_component add shell {
	frame $itk_interior.shell -relief ridge -bd 2
    } {
	keep -background -cursor -width -height
    }
    pack $itk_component(shell) -fill both -expand yes
    
    itk_component add pl {
	Plplotwin $itk_component(shell).pl
    } {
	usual
    }

    pack $itk_component(pl) -fill both -expand yes

    ##
    # Explicitly handle configs that may have been ignored earlier.
    #
    eval itk_initialize $args

}

itcl::body Pltimeseries::initialise {} {
    cmd plcol 15
    cmd plenv $min_time $max_time $y_min $y_max 0 1
    cmd pllab $itk_option(-xname) $itk_option(-yname) $itk_option(-title)
    cmd plcol 1
}

itcl::body Pltimeseries::destructor {} {
}

itcl::body Pltimeseries::tick {} {
    incr _tick
    lappend _ticks $_tick
    if {$_tick <= $max_time} {
	if {$_replot == 1 && (($auto_scale_intelligently && $min_time == 0) \
	  || !$auto_scale_intelligently)} {
	    set _replot 0
	    plotaxes
	    set l 0
	} else {
	    set l [expr {[llength $_ticks] -2}]
	    if {$_replot > 0} {
		incr _replot -1
	    }
	}
	foreach trace [array names _points] {
	    cmd plcol [expr {1 + int($trace * $colour_increment) % 15}]
	    plotline [lrange $_ticks $l end] [lrange $_points($trace) $l end]
	}
	return
    }
    incr min_time $scroll_every
    incr max_time $scroll_every
    set i 0 
    while {[lindex [lindex $_ticks $i] 0] < $min_time} { incr i }
    foreach trace [array names _points] {
	set _points($trace) [lrange $_points($trace) $i end]
    }
    set _ticks [lrange $_ticks $i end]
    plotaxes
    foreach trace [array names _points] {
	cmd plcol [expr {1 + int($trace * $colour_increment) % 15}]
	plotline $_ticks $_points($trace)
    }
}

itcl::body Pltimeseries::plotaxes {} {
    cmd plcol 15
    cmd plenv $min_time $max_time $y_min $y_max 0 1
    cmd pllab $itk_option(-xname) $itk_option(-yname) $itk_option(-title)
    cmd plcol 1
}

itcl::body Pltimeseries::plotpoint {t y} {
    matrix gg f 1 = $t
    matrix hh f 1 = $y
    cmd plsym 1 gg hh 65
}

itcl::body Pltimeseries::plotline {t y} {
    matrix _pt f [llength $t] = $t
    matrix _py f [llength $y] = $y
    cmd plline [llength $t] _pt _py
}

itcl::body Pltimeseries::datapoints {args} {
    for {set i 0} {$i < [llength $args]} {incr i} {
	lappend _points($i) [lindex $args $i]
	if {$auto_scale_freq || $auto_scale_intelligently} {
	    if {[lindex $args $i] > $actual_y_max} {
		set actual_y_max [lindex $args $i]
	    }
	}
    }
    if {$actual_y_max > $y_max} {
	set y_max $actual_y_max
	if {$_replot == 0} {
	    if {$auto_scale_intelligently} {
		set _replot $scroll_every
	    } else {
		set _replot $auto_scale_freq
	    }
	}
    }
    if {$auto_scale_freq || $auto_scale_intelligently} {
	if {$auto_reduce_scale} {
	    if {$actual_y_max < $y_max * $auto_reduce_scale_minimum} {
		set y_max [expr {$actual_y_max/$auto_reduce_scale_minimum}]
		if {$_replot == 0} {
		    if {$auto_scale_intelligently} {
			set _replot $scroll_every
		    } else {
			set _replot $auto_scale_freq
		    }
		}
	    }
	}
    }
}

itcl::body Pltimeseries::test {} {
    for {set i 0} {$i < 10} {incr i} {
	datapoints [expr rand()]
	tick
    }
}




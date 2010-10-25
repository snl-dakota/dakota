## -*-Tcl-*-
 # ###################################################################
 # 
 #  FILE: "plbarchart.tcl"
 #                                    created: 03/21/1998 {00:21:52 AM} 
 #                                last update: 07/01/2002 {06:32:43 PM} 
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

proc plbarchart {args} {uplevel Plbarchart $args}

itcl::class Plbarchart {
    inherit itk::Widget

    public variable V ""
    common n 0
    private variable nn ""
    public variable ymax 1.0
    public variable bins 10
    public variable minbin 0.0
    public variable maxbin 1.0
    private variable step 0.0
    
    itk_option define -title title Title "plot"
    itk_option define -xname xname Xname "x"

    constructor {args} {}
    destructor {}
    
    method internal_set {var to} {
	set $var $to
    }
    method initialise {}
    method cmd {args} { uplevel 1 $itk_component(pl) cmd $args }
    method datapoints {data}
    method drawAxes {}
}

itcl::configbody Plbarchart::ymax {
    if {$V != ""} {drawAxes}
}

itcl::body Plbarchart::constructor {args} {
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

itcl::body Plbarchart::datapoints {data} {
    if {[info commands _pp$nn] == "_pp$nn"} {
	cmd plcol 0
	cmd plhist [_pp$nn info] _pp$nn $minbin $maxbin $bins 0
	rename _pp$nn {}
    }
    matrix -persist _pp$nn f [llength $data] = $data
    cmd plcol 1
    cmd plhist [_pp$nn info] _pp$nn $minbin $maxbin $bins 0
}

itcl::body Plbarchart::drawAxes {} {
    cmd plcol 15
    cmd plenv [expr {[lindex $V 0] -0.7*$step}] \
      [expr {[lindex $V end] +0.7*$step}] \
      0 $ymax 0 0
    cmd pllab $itk_option(-xname) "frequency" $itk_option(-title)
}

itcl::body Plbarchart::initialise {} {
    incr n
    set nn $n
    set V {}
    set step [expr {($maxbin-$minbin)/double($bins)}]
    for {set i 0} {$i < $bins} {incr i} {
	lappend V [expr {$minbin + double($i)*$step}]
    }
    drawAxes
    matrix -persist _pv$nn f [llength $V] = $V
}

itcl::body Plbarchart::destructor {} {
}




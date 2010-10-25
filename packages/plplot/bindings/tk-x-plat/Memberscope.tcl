# This file is in the public domain.
# Use, copy, sell, rewrite, improve however you like.
# Vince Darley.

package require Itcl

itcl::class Memberscope {
    protected method memberscope {var}

    private common _memberscopeTrace
    private method _memberscopeHelper {var obj name1 name2 op}
}

## 
 # -------------------------------------------------------------------------
 # 
 # "Memberscope::memberscope" --
 # 
 #  Like 'scope', but allows you to use a data member or config option
 #  as a global variable in things like 
 #  
 #    checkbutton ... -variable [memberscope datamember]
 #    checkbutton ... -variable [memberscope -configoption]
 #    
 #  [incr Tcl] normally doesn't allow that, since datamembers aren't
 #  real Tcl variables (well, ok Itcl 3.0 does allow it now).  We 
 #  cheat by making a new variable, and creating a trace on it.  Works 
 #  whether the datamember is a real member, or a configuration option 
 #  (checks for leading '-' to choose which approach to use).
 #  
 #  This code doesn't implement two-way traces.  In other words if 
 #  you modify the datamember or config-option, the corresponding
 #  checkbutton (or whatever) doesn't notice.
 #  
 # --Version--Author------------------Changes-------------------------------
 #    1.0     darley@fas.harvard.edu original
 # -------------------------------------------------------------------------
 ##
itcl::body Memberscope::memberscope {var} {
    set i [info exists _memberscopeTrace($this,$var)]
    if {[string match "-*" $var]} {
	set _memberscopeTrace($this,$var) [uplevel 1 $this cget $var]
    } else {
	set _memberscopeTrace($this,$var) [uplevel 1 set $var]
    }
    if {!$i} {
	set _var [itcl::scope _memberscopeTrace($this,$var)]
	set _code [itcl::code $this _memberscopeHelper $var $this]
	set _tr [trace vinfo $_var] 
	if {[lindex $_tr 0] != "w" || ([lindex $_tr 1] != $_code)} {
	    trace variable $_var w $_code
	}
    }
    return [itcl::scope _memberscopeTrace($this,$var)]
}

itcl::body Memberscope::_memberscopeHelper {var obj name1 name2 op} {
    if {[string match "-*" $var]} {
	$obj configure $var [set ${name1}($name2)]
    } else {
	#puts stdout "@scope $obj [list set $var [set ${name1}($name2)]]"
	@scope $obj [list set $var [set ${name1}($name2)]]
    }
}

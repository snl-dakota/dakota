#----------------------------------------------------------------------------
# Source this file into a working Tk interpreter to run all the Tcl demos
# in a nice window with buttons for each demo you'd like to run.
# 
# This requires the 'Plplotwin' extended widget.  If you don't have that
# widget set up (from the 'tk-x-plat' bindings), then use the
# 'runAllDemos.tcl' script instead.
# 
# Vince Darley
# vince@santafe.edu
# 
#----------------------------------------------------------------------------

if {[catch {file readlink [info script]} path]} {
    set path [info script]
}
cd [file join [file dirname $path] .. tcl]
lappend auto_path [pwd]
catch {package require Plplotter}
Plplotwin .p
grid .p -columnspan 5 -sticky news
grid rowconfigure . 0 -weight 1
for {set i 0} {$i < 5} {incr i} {
    grid columnconfigure . $i -weight 1
}

# turn on pauses
.p cmd plspause 1

button .cexit -text "Quit" -command exit
button .cshell -text "Shell" -command "console show"
button .creload -text "Reload" -command reload
button .ctoggle -text "Toggle Menu" -command toggleMenu

set buttons [list .cexit .cshell .creload .ctoggle]

proc toggleMenu {} {
    if {[.p cget -menulocation] == "menubar"} {
	.p configure -menulocation local
    } else {
	.p configure -menulocation menubar
    }
}

proc reload {} {
    global demos
    foreach demo $demos {
	catch {rename $demo {}}
    }
    auto_reset
}

proc run {demo} {
    setButtonState disabled
    .p bop
    update idletasks
    if {[catch {$demo .p} err]} {
	puts stderr $err
    }
    .p bop
    setButtonState normal
}

proc setButtonState {state} {
    foreach b [info commands .b*] {
	$b configure -state $state
    }
}

for {set i 1} {$i <= [llength [glob x*.tcl]]} {incr i} {
    set demo x[format "%02d" $i]
    button .b$i -text "Demo $i" -command [list run $demo]
    lappend demos $demo
    lappend buttons .b$i
    if {[llength $buttons] == 5} {
	eval grid $buttons -sticky ew
	set buttons {}
    }
}

if {[llength $buttons]} {
    eval grid $buttons -sticky ew
}

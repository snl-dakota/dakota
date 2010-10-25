if {[info exists ::tcl_platform(debug)]} {
    set file [file join $dir plplotter521g[info sharedlibextension]]
} else {
    set file [file join $dir plplotter521[info sharedlibextension]]
}

# This little helper is needed to deal seamlessly with the
# possibility that either or both $dir and $file may contain
# spaces, or characters like []{}
proc loadPlplot {dir file} {
    global pllibrary
    set pllibrary $dir
    load $file Plplotter
    rename loadPlplot {}
# put core tcl scripts in path
    lappend auto_path $dir/tcl
}

package ifneeded Plplotter 5.2.1 [list loadPlplot $dir $file]
unset file
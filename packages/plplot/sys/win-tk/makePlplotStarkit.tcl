puts stdout "Making plplotter starkit" ; update
set execdir E:/Apps
catch {
    console show
    wm withdraw .
    update
}
set vers "5.2.1"
cd [file dirname [info script]]
catch {file delete plplotter.kit}
file delete -force plplotter.vfs
file mkdir plplotter.vfs
file mkdir plplotter.vfs/lib
file mkdir plplotter.vfs/lib/plplotter${vers}
file mkdir plplotter.vfs/examples
file mkdir plplotter.vfs/examples/tcl
file mkdir plplotter.vfs/examples/tk
foreach pat {*.tcl tclIndex *.dat *.log} {
    foreach f [glob -dir ../../examples/tcl $pat] {
	file copy $f plplotter.vfs/examples/tcl
    }
}
foreach f [glob -dir ../../examples/tk *.tcl] {
    file copy $f plplotter.vfs/examples/tk
}
foreach f [glob -dir ../../data *.{map,fnt}] {
    file copy $f plplotter.vfs/lib/plplotter${vers}
}
foreach f [glob -dir ../../bindings/tk pl{color,plot,defaults,tools,widget}.tcl] {
    file copy $f plplotter.vfs/lib/plplotter${vers}
}
foreach f [glob -dir ../../bindings/tk-x-plat *.tcl] {
    file copy $f plplotter.vfs/lib/plplotter${vers}
}
file copy pkgIndex.tcl plplotter.vfs/lib/plplotter${vers}
file copy tclIndex plplotter.vfs/lib/plplotter${vers}
foreach f [glob *.dll] {
    file copy $f plplotter.vfs/lib/plplotter${vers}
}

# Make our 'main.tcl'
set fout [open plplotter.vfs/main.tcl w]
puts $fout "set dir \[file normalize \[file dirname \[info script\]\]\]"
puts $fout "lappend auto_path \[file join \$dir lib\]"
puts $fout "source \[file join \$dir examples tk runAllDemos.tcl\]"
close $fout
exec $execdir/tclkitsh $execdir/sdx.kit wrap plplotter.kit -writable
#file delete -force plplotter.vfs
puts "Done making plplotter.kit"

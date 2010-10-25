# configure.tcl --
#     Simple configuration script for the Windows version of
#     PLplot. It recognises the following options:
#     --with-double           Use double precision reals (default)
#     --without-double        Use single precision reals instead
#     --with-single           Use single precision reals instead
#     --prefix=directory      Directory to install PLplot in
#     --installdir=directory  Directory to install PLplot in
#
#     (more to come)
#
#     The script can be run via Tcl/Tk (see the README file
#     for details).
#
#     The user-interface allows you to set the various options.
#     When exiting the program via the "Configure" button,
#     the current values are stored and appropriate changes
#     are made to the various files to reflect the choices:
#     - The option for double or single precision influences
#       the Plconfig.h header file as well as the Fortran
#       examples
#
#     The chosen options are stored in a file "config.res"
#     for future use (if this file exists, it is read at
#     start-up).
#

# mainWindow --
#     Define the main window
# Arguments:
#     None
# Result:
#     None
# Side effect:
#     The main window is filled with the various options
#
proc mainWindow {} {

    global options

    wm title . "PLplot - Windows version"
    wm protocol . WM_DELETE_WINDOW exit
    #
    # Set up the (simple) menu bar
    #
    set  mw    .menu
    menu       $mw
    menu       $mw.file  -tearoff false
    menu       $mw.help  -tearoff false

    $mw add cascade -label File   -menu $mw.file -underline 0
    $mw add cascade -label Help   -menu $mw.help -underline 0

    . configure -menu $mw

    #
    # Set up the "File" menu
    #
    $mw.file add command -label "Configure" -underline 0 \
       -command [list runConfiguration]
    $mw.file add separator
    $mw.file add command -label Exit -underline 1 \
       -command exit

    $mw.help add command -label "About ..." -underline 1 \
       -command aboutInfo

    frame .f ;#-borderwidth 2 -relief raised

    label       .f.install_txt -text "Install in:"
    entry       .f.install_dir -textvariable options(install)
    button      .f.install_sel -text "Browse" -command {selectDir install} -width 10

    label       .f.precision -text "Type of floating-point data:"
    radiobutton .f.double -value "double" -variable options(float) -text "Double precision"
    radiobutton .f.single -value "single" -variable options(float) -text "Single precision"
    label       .f.empty1 -text " "

    grid              .f.install_txt .f.install_dir .f.install_sel
    grid configure    .f.install_txt -sticky nw
    grid configure    .f.install_sel -padx   3
    grid rowconfigure .f 0 -pad 4

    grid              .f.precision -        -
    grid              .f.double .f.single -
    grid configure    .f.precision -sticky nw
    grid rowconfigure .f 1 -pad 4


    button      .configure -text "Configure" -width 10 -command runConfiguration
    button      .cancel    -text "Cancel"    -width 10 -command exit

    label .explain -text "Available options:" -font "Helvetica 10 bold"
    label .empty2  -text " "

    grid  .explain   - -       -
    grid configure .explain -sticky nw
    grid  .f         - -       -
    grid  .empty2    - -
    grid  .configure - .cancel
}

# aboutInfo --
#    Show a simple message box
#
# Arguments:
#    None
# Result:
#    None
# Side effects:
#    Message box shown
#
proc aboutInfo {} {

    tk_messageBox -icon info -type ok -message \
"PLplot - Windows version

Configuration Tool

For more information:
see the README and INSTALL files"
}

# saveConfiguration --
#    Save the configuration data
#
# Arguments:
#    None
# Result:
#    None
# Side effects:
#    Options saved to file for future reference
#
proc saveConfiguration {} {
    global savefile
    global options

    set outfile [open $savefile w]
    puts $outfile "array set options [list [array get options]]"

    close $outfile
}

# runConfiguration --
#    Save the configuration data
#
# Arguments:
#    None
# Result:
#    None
# Side effects:
#    Adjust the various files according to the options
#
proc runConfiguration {} {

    #
    # Save the options
    #
    saveConfiguration

    #
    # Adjust the makefile for installing the files
    #
    adjustFile "makefile.in" "makefile"

    #
    # Adjust the Fortran examples
    #
#   set orgdir [pwd]
#   cd ../../../examples/f77
#
    foreach f [glob *.fm4] {
        adjustFile $f "[file root $f].f"
    }
#   cd $orgdir

    #
    # Put up a message ...
    #
    tk_messageBox -title "PLplot configuration completed" \
        -type ok \
        -message "
Configuration is complete.
Run the makefiles to create the actual
libraries and examples"

    exit
}

# selectDir --
#    Select a new directory
#
# Arguments:
#    which        Which element of the options array to use
# Result:
#    None
# Side effects:
#    Options array contains the new directory
#
proc selectDir {which} {
    global options

    set newdir [tk_chooseDirectory -initialdir $options($which) \
                    -parent . \
                    -mustexist 0 -title "Directory for installation"]
    if { $newdir != "" } {
        set options($which) [file nativename $newdir]
    }
}

# adjustFile --
#    Adjust the contents of the given file and copy it to a new file
#
# Arguments:
#    srcfile         Name of the original file
#    dstfile         Name of the new file
# Result:
#    None
# Side effects:
#    New file written with the new contents
#
proc adjustFile {srcfile dstfile} {
    global options

    #
    # Construct the set of old/new strings
    #
    set replaces {}

    #
    # Single-precision?
    #
    if { $options(float) == "single" } {
        lappend replaces "~define_plflt~" "#undef PL_DOUBLE"
        lappend replaces "real*8"         "real*4"
    } else {
        lappend replaces ~define_plflt~ "#define PL_DOUBLE"
    }

    #
    # Installation directory
    #
    lappend replaces "~installdir~" "$options(install)"

    #
    # Read the file, replace the strings and write the new one
    #
    set infile  [open $srcfile r]
    set outfile [open $dstfile w]

    set contents  [read $infile]
    puts $outfile [string map $replaces $contents]

    close $infile
    close $outfile
}

# handleCmdOptions --
#    Interpret the command-line options - if any
#
# Arguments:
#    arglist         List of command-line options
# Result:
#    None
# Side effects:
#    Set the values in the options array
#
proc handleCmdOptions {arglist} {
    global options

    foreach a $arglist {
        switch -glob -- $a {
            "--with-double"    { set options(float) "double" }
            "--without-double" { set options(float) "single" }
            "--with-single"    { set options(float) "single" }
            "--prefix=*"       -
            "--installdir=*"   {
               set dirname [lindex [split $a "="] 1]
               set options(install) $dirname
            }

            default {
                # Nothing further at the moment
            }
        }
    }
}

# main --
#     Initialise all options and get the GUI started
#
#
set savefile         "config.res"
set options(float)   "double"
set options(install) "c:\\plplot"

if { [file exists $savefile] } {
    source $savefile
}
handleCmdOptions $::argv

mainWindow

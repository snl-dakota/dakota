#----------------------------------------------------------------------------
# $Id: plot.tcl 3186 2006-02-15 18:17:33Z slbrow $
#
# Interactive plotting driver for pltcl (plplot-enhanced tclsh)
# 
# Maurice LeBrun
# 10/18/95
# DejaNews, Inc.
#
# Generates line plot(s) given a file of ASCII data (similar to the GNUPLOT
# "plot" command).  Can be easily modified to particular needs.
#
# To run this demo execute pltcl.  Then simply type 'plot <datafile>'.
# For other options see the usage section below.
#
# Note: this relies on Tcl autoloading to automatically find & source the
# plot.tcl source file where the Tcl plot proc is contained.  If for some
# reason autoloading isn't working right, you'll have to run 'source
# <PATH>plot.tcl' yourself (from pltcl).
#
#----------------------------------------------------------------------------
# Usage:
#
#	plot [data file [columns]]
#
# Examples:
#
#	plot stats.log {1 4}
#
# would plot columns 1 & 4 (y) vs column 0 (x).
#
#       plot stats.log
#
# would plot columns 1,2,3,4 (y) vs column 0 (x)
#
#       plot r.dat
#       plot plot.dat
#
# give illustrative plots with other example data sets.

# The layout of the data file is as follows:
#
# <comment section, may include plotting options>
# <x>  <y1>  <y2> ... <yn>
# [etc]
#
# The number of dependent vars to plot defaults to the number found in the
# data file, up to a maximum of 10 (you can plot more by specifying ny in
# the call).  When there are more than one, they are plotted in different
# colors (chosen from cmap0).
#
# Two powerful mechanisms are used to allow customization of the output.
# First, local plotting options may be specified in the data file itself.
# Comment lines of the form "# set <foo> <bar>" are stripped of their
# leading "# " and evaluated.  The local plot options are given below.
#
# Second, a global associative array -- plopts -- can be used to set the
# local plotting options as well.  E.g. to set "xlab", set plopt(xlab) to
# the desired value at global scope.  Values specified this way take
# precedence over those set in the data file in order to allow interactive
# customization without having to edit the data file.
#
# Local plot options are as follows:
#
#	Name	Meaning		Default value
#	----	-------		-------------
#	file	data file	(none) [1]
#	title	plot title	(none)
#	xlab	x-axis label	x
#	ylab	y-axis label	y
#	xopt	x axis options	bcnst (passed to plbox)
#	yopt	y axis options	bcnstv (same)
#	lsty	line style	1 [2]
#	poin	point symbol	-1 or 1, depending on value of lsty [3]
#	xmin	min x value	(data minimum - x)
#	xmax	max x value	(data maximum - x)
#	ymin	min y value	(data minimum - y)
#	ymax	max y value	(data maximum - y)
#	nfilt	filter points	0 (no filtering)
#	dx	x scale value	1
#
# control parameters:
#
#	noeop - if set, the plot is not ended (so you can overlay stuff)
#
# Notes:
# 1. The data file MUST be specified, either through a global variable
#    or via the invocation line.  Otherwise we return with an error.
#
# 2. Line styles are as follows:
#	 0 : none (only points plotted)
#	 1 : continuous
#	2-8: built-in plplot broken line pattern
#
# 3. Default is -1, which means do not plot points separately, however, if
#    a line style of -1 is specified, default is 1 (single point).  To see
#    all of the valid point types, refer to the manual.  Here's just a few:
#	0	box
#	1	dot
#	2	cross
#	3	asterisk
#	4	circle
#	5	x
#	6	box (what, again?)
#	7	triangle
#----------------------------------------------------------------------------

proc plot {{file {}} {columns {}}} {
    global plopt

# Make sure there is a data file

    if {$file == ""} {
	if {[info exists plopt(file)]} {
	    set file $plopt(file)
	} else {
	    error "Error: no data file specified"
	}
    }

# Set up plot option defaults

    set title	""
    set xlab	x
    set ylab	y
    set xopt	bcnst
    set yopt	bcnstv
    set lsty	1
    set poin	-1
    set nfilt	0
    set dx	1

# If plinit hasn't been called, do it now.

    plgdev device
    if {$device == ""} {
	plinit
	plgdev device
    }

# Initialize data arrays

    set matlen 128
    set matinc 128
    matrix x f $matlen

    set nx 0
    set first_time 1

# Open file and read into data arrays

    set fileid [open $file]
    while {[expr [gets $fileid input] > -1]} {
	set first_char [string index [lindex $input 0] 0]

    # Ignore blank lines

	if {$first_char == ""} continue

    # Comments or set commands

	if {$first_char == "\#"} {
	    set nextword [lindex $input 1]
	    if {"$nextword" == "set"} {
		set command [lrange $input 1 end]
		eval $command
	    }
	    continue
	}

    # Now we should be to the data part.  First time through only, allocate
    # sufficient arrays for holding all the dependent data (faster this way
    # than putting too much control logic in this loop).  If you are missing
    # entries later on, this will probably fail.

	if {$first_time} {
	    set first_time 0
	    set nymax [expr [llength $input] - 1]
	    for {set iy 1} {$iy <= $nymax} {incr iy} {
		matrix y$iy f $matlen
	    }
	    if {$columns == {}} {
		set columns 1
		for {set iy 2} {$iy <= $nymax} {incr iy} {
		    set columns "$columns $iy"
		}
	    }
	}

    # Increase array bounds if necessary

	if {$nx == $matlen} {
	    incr matlen $matinc
	    x redim $matlen
	    for {set iy 1} {$iy <= $nymax} {incr iy} {
		y$iy redim $matlen
	    }
	}

    # Read data into arrays

	x $nx = [lindex $input 0]
	for {set iy 1} {$iy <= $nymax} {incr iy} {
	    y$iy $nx = [lindex $input $iy]
	}
	incr nx
    }
    close $fileid

    set ny [llength $columns]

    puts "Read $nx data points, $ny separate streams"

# Filter if necessary

    if {$nfilt != 0} {
	for {set iy 0} {$iy < $ny} {incr iy} {
	    set jy [lindex $columns $iy]
	    y$jy filter $nfilt
	}
    }

# Get data min/max

    if {![info exists xmin]} {
	set xmin [x min $nx]
    }

    if {![info exists xmax]} {
	set xmax [x max $nx]
    }

    if {![info exists ymin]} {
	set jy [lindex $columns 0]
	set ymin [y$jy min $nx]
	for {set iy 1} {$iy < $ny} {incr iy} {
	    set jy [lindex $columns $iy]
	    set y1min [y$jy min $nx]
	    if {[expr $y1min < $ymin]} then {set ymin $y1min}
	}
    }

    if {![info exists ymax]} {
	set jy [lindex $columns 0]
	set ymax [y$jy max $nx]
	for {set iy 1} {$iy < $ny} {incr iy} {
	    set jy [lindex $columns $iy]
	    set y1max [y$jy max $nx]
	    if {[expr $y1max < $ymax]} then {set ymax $y1max}
	}
    }

# Rescale if necessary

    if {$dx != 1} {
	x scale $dx
	set xmin [expr $xmin * $dx]
	set xmax [expr $xmax * $dx]
    }

# Set up plot options

    if {[info exists plopt(title)]}	{set title $plopt(title)}
    if {[info exists plopt(xlab)]}	{set xlab $plopt(xlab)}
    if {[info exists plopt(ylab)]}	{set ylab $plopt(ylab)}
    if {[info exists plopt(xopt)]}	{set xopt $plopt(xopt)}
    if {[info exists plopt(yopt)]}	{set yopt $plopt(yopt)}
    if {[info exists plopt(lsty)]}	{set lsty $plopt(lsty)}
    if {[info exists plopt(poin)]}	{set poin $plopt(poin)}
    if {[info exists plopt(xmin)]}	{set xmin $plopt(xmin)}
    if {[info exists plopt(xmax)]}	{set xmax $plopt(xmax)}
    if {[info exists plopt(ymin)]}	{set ymin $plopt(ymin)}
    if {[info exists plopt(ymax)]}	{set ymax $plopt(ymax)}
    if {[info exists plopt(nfilt)]}	{set nfilt $plopt(nfilt)}
    if {[info exists plopt(dx)]}	{set dx $plopt(dx)}
    if {[info exists plopt(noeop)]}	{set noeop $plopt(noeop)}

# Set up the plot

    plbop

    plvpor 0.15 0.85 0.1 0.9
    plwind $xmin $xmax $ymin $ymax

    plcol 1
    pllsty 1
    plbox $xopt 0.0 0 $yopt 0.0 0

    plcol 2
    pllab "$xlab" "$ylab" "$title"

# Plot the data

    if { $lsty > 0 } {
	pllsty $lsty
	for {set iy 0} {$iy < $ny} {incr iy} {
	    set jy [lindex $columns $iy]
	    plcol [expr 2 + $jy]
	    plline $nx x y$jy
	}

    } else {
	if { $poin < 0 } {set poin 1}
	for {set iy 0} {$iy < $ny} {incr iy} {
	    set jy [lindex $columns $iy]
	    plcol [expr 2 + $jy]
	    plpoin $nx x y$jy $poin
	}
    }

# End the page (see note about pause above).

    pllsty 1

    if {[info exists noeop] == 0} pleop
}

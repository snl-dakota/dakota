## -*-Tcl-*-
 # ###################################################################
 # 
 # FILE: "Plplotwin.tcl"
 #                                     created: 18/6/96 {2:06:08 pm} 
 #                             last update: 07/01/2002 {03:40:29 PM}
 # Author: Vince Darley, based upon ordinary Tcl code written
 #                       by Geoffrey Furnish (and possibly others)
 # E-mail: <vince@santafe.edu>
 # mail: 317 Paseo de Peralta, Santa Fe, NM 87501
 # www: <http://www.santafe.edu/~vince/>
 # 
 # Description:
 # 
 # A unified Itk 'extended' widget for Plplot.
 # 
 # Derives from the plain Plwindow widget which wraps a plplot driver
 # 
 # Attached menus can be configured in a variety of ways:
 # (i) Long Tk 8 cross-platform menubar
 # (ii) Compressed Tk 8 cross-platform menubar (all items in 1 menu)
 # (iii) Like (ii) but in a menubutton to left of statusbar
 # (iv) Placed in any menu you pass to the widget
 # Use '$widget configure -menulocation' to adjust this placing.
 # 
 # Grabbed all the old legacy code and put it together in one
 # piece. Should be much more clear what's going on now. It has
 # also all been updated for Itk 3.0, using that package's mega-
 # widget capability.
 # 
 # August/Sept'99 fixed a bunch of subtle problems which can arise
 # due to using low tcl_precision, and with Tk's event loop, also
 # adjusted various 'grid' commands so unnecessary resizing doesn't
 # happen.  Added optional 'wait' argument to some zooming code
 # for many fewer redraws.  Fixed buggy plSaveFile, and added new
 # filter for .ppm
 # 
 # November '99.  Fixed propagation problems so zooming etc won't
 # change geometry of parent and cause all sorts of weird problems
 # for the windows inside which we've packed this item.  Also fixed
 # various 'itk::usual' problems so this can easily form a subwidget
 # inside e.g. Pltimeseries, Plbarchart...
 # 
 # modified by rev reason
 # -------- --- --- -----------
 # 19/7/96 VMD 1.0 original - many minor bugs probably exist.
 # 21/1/97 VMD 1.1 various small fixes, plus uses 'grid' not 'pack' now
 # 1999-09-02 VMD Vast number of improvements, bug fixes in 1997-1999.
 # ###################################################################
 ##

package require Tk 8.0
package require Plplotter
package require Itk 3.0

#
# Usual options.
#
itk::usual Plplotwin {
    keep -menulocation -zoomfromcenter -zoompreservesaspect \
      -background -cursor -foreground \
      -plotbackground -xhairs -doublebuffer 
}

itcl::class Plplotwin {
    inherit Plwindow Memberscope

    constructor {args} {}
    destructor {}
    
    itk_option define -menulocation menuLocation Menulocation "menubar"
    itk_option define -zoomfromcenter zoomFromCenter ZoomFromCenter 1
    itk_option define -zoompreservesaspect zoomPreservesAspect ZoomPreservesAspect 0
    
    protected method plwin {} { return $itk_interior.plwin }
    
    public method setup_defaults {} {}
    public method start {} {}
    public method key_filter {keycode state x y keyname ascii} {}
    public method user_key {keycode state x y keyname ascii} {}
    public method user_mouse {button state x y} {}
    public method flash {col} {}
    public method end {} {}
    public method print {} {}
    public method save_as {{file ""}} {}
    public method save_again {} {}
    public method save_close {} {}
    public method update_zoom {} {}
    public method zoom_select {} {}
    public method zoom_enter {} {}
    public method zoom_reset {} {}
    public method update_orient {} {}
    public method orient {rot} {}
    public method page_enter {} {}
    public method page_reset {} {}
    public method zoom_start {wx wy} {}
    public method zoom_coords {x0 y0 x1 y1 opt} {}
    public method zoom_mouse_draw {wx0 wy0 wx1 wy1} {}
    public method zoom_mouse_end {wx0 wy0 wx1 wy1} {}
    public method view_select {x0 y0 x1 y1} {}
    public method view_zoom {x0 y0 x1 y1} {}
    public method zoom_back {} {}
    public method zoom_forward {} {}
    public method view_scroll {dx dy s} {}
    public method update_view {} {}
    public method label_reset {} {}
    public method label_set {msg} {}
    public method dplink {client} {}
    public method status_msg {msg} {}
    public method next_page {} {}
    public method eop {} {}
    public method bop {} {}
   
    protected method create_pmenu {} {}
    protected method create_pmenu_file {} {}
    protected method create_pmenu_orient {} {}
    protected method create_pmenu_zoom {} {}
    protected method create_pmenu_page {} {}
    protected method create_pmenu_redraw {} {}
    protected method create_pmenu_options {} {}
    protected method fixview {hscroll vscroll} {}
    
    protected method _menuLocate {where} {}
    protected method _menuDestroy {} {}
    protected method menuPath {{sub ""}} {}
    
    private variable zidx 0
    private variable zidx_max 0
    private variable zcoords
    
    private variable menulocation ""
    private variable menulocationtype ""

    private common saveOpts
}


itcl::body Plplotwin::constructor {args} {
    # Set up defaults
    
    setup_defaults 
    
    itk_component add topframe {
	frame $itk_interior.tf -relief ridge
    }
    
    itk_component add lstat {
	label $itk_component(topframe).lstat -anchor w -relief raised
    } 

    # Plframe widget must already have been created (the plframe is queried 
    # for a list of the valid output devices for page dumps).
    
    grid $itk_interior.tf -row 0 -columnspan 2 -sticky nsew
    grid columnconfigure $itk_interior 0 -weight 1 -minsize 0
    grid rowconfigure $itk_interior 0 -weight 0 -minsize 0
    # put the label in the topframe
    grid $itk_component(lstat) -column 1 -sticky nsew
    grid columnconfigure $itk_interior.tf 1 -weight 1 -minsize 0
    
    eval itk_initialize $args
    
    label_reset 
    update idletasks
    # This is necessary so that if we try to insert a large
    # label in the status bar, it doesn't make the whole
    # widget expand, which then causes the plot to redraw,...
    # Similarly for when scrollbars appear etc.
    grid propagate $itk_interior 0
    
    [plwin] configure -eopcmd [itcl::code $this eop] -bopcmd [itcl::code $this bop]
    bind [plwin] <Button> [itcl::code $this next_page]
    
    # Grab the initial input focus.
    focus [plwin]
}

itcl::body Plplotwin::destructor {} {
    $this configure -menulocation ""
}

# EXTREMELY IMPORTANT.  This procedure must not re-enter
# Tcl's event loop.  You must not call 'update', 'update idletasks'
# or anything like that.  A future version of the plframe may avoid
# this problem, but right now it can cause horrible problems when
# you have two or more plframes both of which need to be updated
# simultaneously.  If you have 'update idletasks', the when the
# plframe calls this method (in the middle of updating itself),
# Tcl will give the other frame a chance to update too.  This 
# leads to horrible confusion, involving one window drawing all
# over the other.
itcl::body Plplotwin::eop {} {
    label_set "Hit return or click on the plot for the next page."
}

# EXTREMELY IMPORTANT.  This procedure must not re-enter
# Tcl's event loop.  You must not call 'update', 'update idletasks'
# or anything like that.  A future version of the plframe may avoid
# this problem, but right now it can cause horrible problems when
# you have two or more plframes both of which need to be updated
# simultaneously.  If you have 'update idletasks', the when the
# plframe calls this method (in the middle of updating itself),
# Tcl will give the other frame a chance to update too.  This 
# leads to horrible confusion, involving one window drawing all
# over the other.
itcl::body Plplotwin::bop {} {
    label_set ""
}

itcl::configbody Plplotwin::menulocation {
    if {![info exists menulocationtype]} {
	set menulocationtype ""
	set menulocation ""
    }
    if {$menulocationtype != $itk_option(-menulocation)} {
	catch {_menuDestroy}
	if {$itk_option(-menulocation) != ""} {
	    _menuLocate $itk_option(-menulocation)
	}
    }
}

itcl::body Plplotwin::_menuDestroy {} {
    switch -- $menulocationtype {
	"local" {
	    destroy $itk_component(topframe).mb
	    destroy $itk_component(topframe).mb.menu
	}
	"menubar" {
	    [winfo toplevel $itk_interior] configure -menu ""
	    destroy $itk_interior.pmenu
	}
	"menubarcollapsed" {
	    [winfo toplevel $itk_interior] configure -menu ""
	    destroy $itk_interior.pmenu.m
	}
	default {
	    if {$menulocationtype == ""} {return}
	    if {[winfo exists $menulocationtype]} {
		destroy $menulocationtype
	    }
	}
    }
    set menulocationtype ""
    set menulocation ""
}

itcl::body Plplotwin::_menuLocate {where} {
    set menulocationtype $where
    switch -- $where {
	"local" {
	    menubutton $itk_component(topframe).mb -text "Plot" \
	      -menu $itk_component(topframe).mb.menu -relief raised
	    set menulocation [menu $itk_component(topframe).mb.menu]
	    grid $itk_component(topframe).mb -column 0 -row 0 -sticky nsw
	}
	"menubar" {
	    set menulocation [menu $itk_interior.pmenu]
	    [winfo toplevel $itk_interior] configure -menu $itk_interior.pmenu
	}
	"menubarcollapsed" {
	    menu $itk_interior.pmenu
	    [winfo toplevel $itk_interior] configure -menu $itk_interior.pmenu
	    set menulocation [menu $itk_interior.pmenu.m]
	    $itk_interior.pmenu add cascade -label "Plot" -menu $itk_interior.pmenu.m
	}
	default {
	    if {![winfo exists $where]} {
		error "Bad menu location '$where'; must be 'local',\
		  'menubar', 'menubarcollapsed' or the name of an existing menu."
	    }
	    set menulocation $where
	}
    }
    create_pmenu
}

itcl::body Plplotwin::menuPath {{sub ""}} { 
    set base $menulocation
    if {$sub == ""} { 
	return $base 
    } else {
	return $base.$sub
    }
}

#----------------------------------------------------------------------------
# setup_defaults
#
# Set up default settings.
#----------------------------------------------------------------------------

itcl::body Plplotwin::setup_defaults {} {
    # Set up zoom windows list
    
    set zidx 0
    set zidx_max 0
    set zcoords(0) [list 0.0 0.0 1.0 1.0]
    
    set saveOpts($this,savedevice) "psc"
    set saveOpts($this,savemanyplotsperfile) 0
    set saveOpts($this,flipBWforSave) 1

    # Bindings
    
    bind [plwin] <Any-KeyPress> \
      [itcl::code $this key_filter %N %s %x %y %K %A]
    
    bind [plwin] <Any-ButtonPress> \
      [itcl::code $this user_mouse %b %s %x %y]
    
    bind [plwin] <Any-Enter> \
      "focus [plwin]"
}


#----------------------------------------------------------------------------
# create_pmenu
#
# Create plot menu.
#
# It is tempting to create buttons for some of these options, but buttons
# are difficult to effectively place and extend. Menus have a clear
# placement mechanism and are easy to add to. Further, TK menus can be
# torn off (select menu with middle mouse button and move to where you
# want it) which makes selecting top-level menu buttons easy. Finally,
# certain menu options have keyboard equivalents: zoom-select (z),
# zoom-reset (r), print (P), and save-again (s).
#----------------------------------------------------------------------------

itcl::body Plplotwin::create_pmenu {} {
    create_pmenu_file 
    create_pmenu_orient 
    create_pmenu_zoom 
    create_pmenu_page 
    create_pmenu_options 
}

#----------------------------------------------------------------------------
# create_pmenu_file
#
# Create plot-file menu (cascade)
#----------------------------------------------------------------------------

itcl::body Plplotwin::create_pmenu_file {} {
    set m [menuPath file]
    
    [menuPath] add cascade -label "File" -menu $m
    menu $m
    
    #$m add command -label "Print..." \
      -command [itcl::code $this print] -accelerator Cmd-P
    # Save - As
    
    $m add command -label "Save As" \
      -command [itcl::code $this save_as ]
    
    # Save - Again
    
    $m add command -label "Save Again" \
      -command [itcl::code $this save_again ] \
      -state disabled
    
    # Save - Close
    
    $m add command -label "Save Close" \
      -command [itcl::code $this save_close ] \
      -state disabled
    
    $m add separator
    
    # Save - Set device.. (another cascade)
    
    $m add cascade -label "Save device" -menu $m.sdev
    menu $m.sdev
    
    $m.sdev add check -label "Flip B/W before save or print" \
      -variable [itcl::scope saveOpts($this,flipBWforSave)]
    $m.sdev add separator

    # Generate the device list in the "Save/Set device" widget menu, by querying
    # the plframe widget for the available output devices (which are listed).
    
    set devnames [[plwin] info devnames]
    set devkeys [[plwin] info devkeys]
    set ndevs [llength $devnames]
    for {set i 0} {$i < $ndevs} {incr i} {
	set devnam [lindex $devnames $i]
	set devkey [lindex $devkeys $i]
	
	$m.sdev add radio -label $devnam \
	  -variable [itcl::scope saveOpts($this,savedevice)] -value $devkey
    }
    
    # Save - Set file type.. (another cascade)
    
    $m add cascade -label "Set file type" -menu $m.sfile
    menu $m.sfile
    
    # Single file (one plot/file)
    
    $m.sfile add radio -label "Single file (one plot/file)" \
      -variable [itcl::scope saveOpts($this,savemanyplotsperfile)] -value 0
    
    # Archive file (many plots/file)
    
    $m.sfile add radio -label "Archive file (many plots/file)" \
      -variable [itcl::scope saveOpts($this,savemanyplotsperfile)] -value 1
}

#----------------------------------------------------------------------------
# create_pmenu_orient
#
# Create plot-orient menu (cascade)
#----------------------------------------------------------------------------

itcl::body Plplotwin::create_pmenu_orient {} {
    set m [menuPath orient]
    
    [menuPath] add cascade -label "Orient" -menu $m
    menu $m
    
    $m configure -postcommand [itcl::code $this update_orient ]
    
    # Orient - 0 degrees
    
    $m add radio -label "0 degrees" \
      -command [itcl::code $this orient 0]
    
    # Orient - 90 degrees
    
    $m add radio -label "90 degrees" \
      -command [itcl::code $this orient 1]
    
    # Orient - 180 degrees
    
    $m add radio -label "180 degrees" \
      -command [itcl::code $this orient 2]
    
    # Orient - 270 degrees
    
    $m add radio -label "270 degrees" \
      -command [itcl::code $this orient 3]
}

#----------------------------------------------------------------------------
# create_pmenu_zoom
#
# Create plot-zoom menu (cascade)
#----------------------------------------------------------------------------

itcl::body Plplotwin::create_pmenu_zoom {} {
    set m [menuPath zoom]
    
    [menuPath] add cascade -label "Zoom" -menu $m
    menu $m
    
    $m configure -postcommand [itcl::code $this update_zoom ]
    
    # Zoom - select (by mouse)
    
    $m add command -label "Select" \
      -command [itcl::code $this zoom_select ]
    
    # Zoom - back (go back 1 zoom level)
    
    $m add command -label "Back" \
      -command [itcl::code $this zoom_back ] \
      -state disabled
    
    # Zoom - forward (go forward 1 zoom level)
    
    $m add command -label "Forward" \
      -command [itcl::code $this zoom_forward ] \
      -state disabled
    
    # Zoom - enter bounds
    
    $m add command -label "Enter bounds.." \
      -command [itcl::code $this zoom_enter ]
    
    # Zoom - reset
    
    $m add command -label "Reset" \
      -command [itcl::code $this zoom_reset ]
    
    # Zoom - options (another cascade)
    
    $m add cascade -label "Options" -menu $m.options
    menu $m.options
    
    $m.options add check -label "Preserve aspect ratio" \
      -variable [memberscope -zoompreservesaspect]
    
    $m.options add separator
    
    $m.options add radio -label "Start from corner" \
      -variable [memberscope -zoomfromcenter] \
      -value 0

    $m.options add radio -label "Start from center" \
      -variable [memberscope -zoomfromcenter] \
      -value 1
    
    $m.options invoke 1
}

#----------------------------------------------------------------------------
# create_pmenu_page
#
# Create plot-page menu (cascade)
#----------------------------------------------------------------------------

itcl::body Plplotwin::create_pmenu_page {} {
    set m [menuPath page]
    
    [menuPath] add cascade -label "Page" -menu $m
    menu $m
    
    # Page - enter bounds
    
    $m add command -label "Setup.." \
      -command [itcl::code $this page_enter ]
    
    # Page - reset
    
    $m add command -label "Reset" \
      -command [itcl::code $this page_reset ]
}

#----------------------------------------------------------------------------
# create_pmenu_redraw
#
# Create plot-redraw menu
# I only use this for debugging in cases where the normal redraw capability
# isn't working right.
#----------------------------------------------------------------------------

itcl::body Plplotwin::create_pmenu_redraw {} {
    [menuPath] add command -label "Redraw" \
      -command "[plwin] redraw"
}

#----------------------------------------------------------------------------
# create_pmenu_options
#
# Create plot-options menu (cascade)
#----------------------------------------------------------------------------

itcl::body Plplotwin::create_pmenu_options {} {
    set m [menuPath options]
    
    [menuPath] add cascade -label "Options" -menu $m
    menu $m
    
    $m add command -label "Palette 0" \
      -command "plcmap0_edit [plwin]" 
    
    $m add command -label "Palette 1" \
      -command "plcmap1_edit [plwin]" 
    
    $m add checkbutton -label "Crosshairs" \
      -variable [memberscope -xhairs]
    
    $m add checkbutton -label "Doublebuffer" \
      -variable [memberscope -doublebuffer]
}

#----------------------------------------------------------------------------
# start
#
# Responsible for plplot graphics package initialization on the widget.
# People driving the widget directly should just use pack themselves.
#
# Put here to reduce the possibility of a time-out over a slow network --
# the client program waits until the variable widget_is_ready is set.
#----------------------------------------------------------------------------

itcl::body Plplotwin::start {} {
    global client
    
    # Manage widget hierarchy
    
    pack $this -side bottom -expand 1 -fill both
    
    update
    
    # Inform client that we're done.
    
    if { [info exists client] } {
	client_cmd "set widget_is_ready 1"
    }
}

#----------------------------------------------------------------------------
# key_filter
#
# Front-end to key handler.
# For supported operations it's best to modify the global key variables
# to get the desired action. More advanced stuff can be done with the
# $user_key_filter proc. Find anything particularly useful? Let me know,
# so it can be added to the default behavior.
#----------------------------------------------------------------------------

itcl::body Plplotwin::key_filter {keycode state x y keyname ascii} {
    global user_key_filter
    
    global key_zoom_select
    global key_zoom_reset
    global key_print
    global key_save_again
    global key_scroll_right
    global key_scroll_left
    global key_scroll_up
    global key_scroll_down
    
    # puts "keypress: $keyname $keycode $ascii $state"
    
    # Call user-defined key filter, if one exists
    
    if { [info exists user_key_filter] } {
	$user_key_filter $keyname $keycode $ascii
    }
    
    # Interpret keystroke
    
    switch $keyname \
      $key_zoom_select [itcl::code $this zoom_select ] \
      "b" [itcl::code $this zoom_back ] \
      "f" [itcl::code $this zoom_forward ] \
      $key_zoom_reset [itcl::code $this zoom_reset ] \
      $key_print [itcl::code $this print ] \
      $key_save_again [itcl::code $this save_again ] \
      $key_scroll_right [itcl::code $this view_scroll 1 0 $state] \
      $key_scroll_left [itcl::code $this view_scroll -1 0 $state] \
      $key_scroll_up [itcl::code $this view_scroll 0 -1 $state] \
      $key_scroll_down [itcl::code $this view_scroll 0 1 $state] \
      Return [itcl::code $this next_page]
    
    # Pass keypress event info back to client.
    
    user_key $keycode $state $x $y $keyname $ascii
}

itcl::body Plplotwin::next_page {} {
    [plwin] nextpage
}

#----------------------------------------------------------------------------
# user_key
#
# Passes keypress event information back to client.
# Based on user_mouse.
#----------------------------------------------------------------------------

itcl::body Plplotwin::user_key {keycode state x y keyname ascii} {
    global client
    
    if { [info exists client] } {
	
	# calculate relative window coordinates.
	
	set xw [expr "$x / [winfo width [plwin]]."]
	set yw [expr "1.0 - $y / [winfo height [plwin]]."]
	
	# calculate normalized device coordinates into original window.
	
	set view [[plwin] view]
	set xrange [expr "[lindex $view 2] - [lindex $view 0]"]
	set xnd [expr "($xw * $xrange) + [lindex $view 0]"]
	set yrange [expr "[lindex $view 3] - [lindex $view 1]"]
	set ynd [expr "($yw * $yrange ) + [lindex $view 1]"]
	
	# send them back to the client.
	
	# puts "keypress $keycode $state $x $y $xnd $ynd $keyname $ascii"
	client_cmd \
	  [list keypress $keycode $state $x $y $xnd $ynd $keyname $ascii]
    }
}

#----------------------------------------------------------------------------
# user_mouse
#
# Passes buttonpress event information back to client.
# Written by Radey Shouman
#----------------------------------------------------------------------------

itcl::body Plplotwin::user_mouse {button state x y} {
    global client
    
    if { [info exists client] } {
	
	# calculate relative window coordinates.
	
	set xw [expr "$x / [winfo width [plwin]]."]
	set yw [expr "1.0 - $y / [winfo height [plwin]]."]
	
	# calculate normalized device coordinates into original window.
	
	set view [[plwin] view]
	set xrange [expr "[lindex $view 2] - [lindex $view 0]"]
	set xnd [expr "($xw * $xrange) + [lindex $view 0]"]
	set yrange [expr "[lindex $view 3] - [lindex $view 1]"]
	set ynd [expr "($yw * $yrange ) + [lindex $view 1]"]
	
	# send them back to the client.
	
	client_cmd \
	  [list buttonpress $button $state $x $y $xnd $ynd]
    }
}

#----------------------------------------------------------------------------
# flash
#
# Set eop button color to indicate page status.
#----------------------------------------------------------------------------

itcl::body Plplotwin::flash {col} {
    $itk_component(ftop).leop config -bg $col
    update idletasks
}

#----------------------------------------------------------------------------
# end
#
# Executed as part of orderly shutdown procedure. Eventually will just
# destroy the plframe and surrounding widgets, and server will exit only
# if all plotting widgets have been destroyed and it is a child of the
# plplot/TK driver. Maybe.
#
# The closelink command was added in the hopes of making the dp driver
# cleanup a bit more robust, but doesn't seem to have any effect except
# to slow things down quite a bit. 
#----------------------------------------------------------------------------

itcl::body Plplotwin::end {} {
    global dp
    # [plwin] closelink
    if { $dp } {
	global list_sock
	close $list_sock
    }
    exit
}

#----------------------------------------------------------------------------
# print
#
# Prints plot. Uses the "plpr" script, which must be set up for your site
# as appropriate. There are better ways to do it but this way is safest
# for now.
#----------------------------------------------------------------------------

itcl::body Plplotwin::print {} {
    label_set "Printing plot..."
    update
    if { [catch "[plwin] print" foo] } {
	bogue_out "$foo"
    } else {
	status_msg "Plot printed."
    }
}

proc plSaveFile {devkey} {
    switch -- "$devkey" \
      "ps"	"set filter .ps" \
      "psc"	"set filter .ps" \
      "plmeta"	"set filter .plm" \
      "pbm"	"set filter .ppm" \
      "xfig"	"set filter .fig"
    
    if {[info exists filter]} {
	set f [tk_getSaveFile -defaultextension $filter]
    } else {
	set f [tk_getSaveFile]
    }
    # the save dialog asked the user whether to replace already.
    if [file exists $f] { file delete $f }
    return $f
}

#----------------------------------------------------------------------------
# save_as
#
# Saves plot to default device, prompting for file name.
#----------------------------------------------------------------------------

itcl::body Plplotwin::save_as {{file ""}} {
    if {$file == ""} {
	set file [plSaveFile $saveOpts($this,savedevice)]
    }
    if { [string length $file] > 0 } {
	label_set "Saving plot..."
	update
	if {$saveOpts($this,flipBWforSave)} {
	    set c0 [cmd plgcmap0]
	    cmd plscmap0 16 #ffffff
	    for {set i 1} {$i <= 15} {incr i} {
		cmd plscol0 $i #000000
	    }
	}
	if { [catch [list [plwin] save as $saveOpts($this,savedevice) $file] foo] } {
	    label_reset
	    bogue_out "$foo"
	} else {
	    status_msg "Plot saved."
	}
	if {$saveOpts($this,flipBWforSave)} {
	    #eval cmd plscmap0 $c0
	    cmd plscmap0 16 #000000
	    for {set i 1} {$i <= 15} {incr i} {
		cmd plscol0 $i [lindex $c0 [expr $i +1]]
	    }	    
	}
	
	if { $saveOpts($this,savemanyplotsperfile) == 0 } {
	    [plwin] save close
	} else {
	    [menuPath file] entryconfigure "Save Again" -state normal
	    [menuPath file] entryconfigure "Save Close" -state normal
	    bogue_out "Warning: archive files must be closed before using"
	}
    } else {
	bogue_out "No file specified"
    }
}

#----------------------------------------------------------------------------
# save_again
#
# Saves plot to an already open file.
#----------------------------------------------------------------------------

itcl::body Plplotwin::save_again {} {
    if { [catch "[plwin] save" foo] } {
	bogue_out "$foo"
    } else {
	status_msg "Plot saved."
    }
}

#----------------------------------------------------------------------------
# save_close
#
# Close archive save file.
#----------------------------------------------------------------------------

itcl::body Plplotwin::save_close {} {
    if { [catch "[plwin] save close" foo] } {
	bogue_out "$foo"
    } else {
	status_msg "Archive file closed."
	[menuPath file] entryconfigure "Save Again" -state disabled
	[menuPath file] entryconfigure "Save Close" -state disabled
    }
}

#----------------------------------------------------------------------------
# update_zoom
#
# Responsible for making sure zoom menu entries are normal or disabled as
# appropriate. In particular, that "Back" or "Forward" are only displayed
# if it is possible to traverse the zoom windows list in that direction.
#----------------------------------------------------------------------------

itcl::body Plplotwin::update_zoom {} {
    # Back
    
    if { $zidx == 0 } {
	[menuPath zoom] entryconfigure "Back" -state disabled
    } else {
	[menuPath zoom] entryconfigure "Back" -state normal
    }
    
    # Forward
    
    if { $zidx_max == 0 || $zidx == $zidx_max } {
	[menuPath zoom] entryconfigure "Forward" -state disabled
    } else {
	[menuPath zoom] entryconfigure "Forward" -state normal
    }
}

#----------------------------------------------------------------------------
# zoom_select
#
# Zooms plot in response to mouse selection.
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_select {} {
    global def_button_cmd 
    
    set def_button_cmd [bind [plwin] <ButtonPress>]
    
    if { $itk_option(-zoomfromcenter) == 0 } {
	label_set "Click on one corner of zoom region."
    } else {
	label_set "Click on center of zoom region."
    }
    
    bind [plwin] <ButtonPress> [itcl::code $this zoom_start %x %y]
}

#----------------------------------------------------------------------------
# zoom_enter
#
# Zooms plot in response to text entry.
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_enter {} {
    global fv00 fv01 fv10 fv11
    global fn00 fn01 fn10 fn11
    
    set coords [[plwin] view]
    
    foreach {fv00 fv01 fv10 fv11} $coords {}
    
    set fn00 xmin
    set fn01 ymin
    set fn10 xmax
    set fn11 ymax
    
    Form2d .e "Enter window coordinates for zoom. Each coordinate\
      should range from 0 to 1, with (0,0) corresponding to the lower\
      left hand corner."
    tkwait window .e
    
    view_select $fv00 $fv01 $fv10 $fv11
}

#----------------------------------------------------------------------------
# zoom_reset
#
# Resets after zoom.
# Note that an explicit redraw is not necessary since the packer issues a
# resize after the scrollbars are unmapped.
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_reset {} {
    global def_button_cmd
    
    label_reset
    bind [plwin] <ButtonPress> $def_button_cmd

    # We require the extra argument 'wait' so that we don't redraw/size
    # the image several times while Tcl is dealing with the scrollbars
    [plwin] view reset wait
    if {[winfo exists $itk_interior.hscroll] && [winfo ismapped $itk_interior.hscroll]} {
	grid forget $itk_interior.hscroll
    }
    if {[winfo exists $itk_interior.vscroll] && [winfo ismapped $itk_interior.vscroll]} {
	grid forget $itk_interior.vscroll
    }
    #grid rowconfigure $itk_interior 2 -weight 0
    #grid columnconfigure $itk_interior 1 -weight 0
    #grid forget $itk_component(plwin)
    #grid $itk_component(plwin) -row 1 -column 0 -sticky nsew
    #grid rowconfigure $itk_interior 1 -weight 1 -minsize 0

    # Reset zoom windows list
    
    set zidx 0
    set zidx_max 0
    set zcoords(0) [list 0.0 0.0 1.0 1.0]
}

#----------------------------------------------------------------------------
# update_orient
#
# Responsible for making sure orientation radio buttons are up to date.
#----------------------------------------------------------------------------

itcl::body Plplotwin::update_orient {} {
    [menuPath orient] invoke "[expr 90*int([[plwin] orient])] degrees"
}

#----------------------------------------------------------------------------
# orient
#
# Changes plot orientation.
#----------------------------------------------------------------------------

itcl::body Plplotwin::orient {rot} {
    if { [[plwin] orient] != $rot} {
	[plwin] orient $rot
    }
}

#----------------------------------------------------------------------------
# page_enter
#
# Changes output page parameters (margins, aspect ratio, justification).
#----------------------------------------------------------------------------

itcl::body Plplotwin::page_enter {} {
    global fv00 fv01 fv10 fv11
    global fn00 fn01 fn10 fn11
    
    set coords [[plwin] page]
    
    foreach {fv00 fv01 fv10 fv11} $coords {}
    
    set fn00 mar
    set fn01 aspect
    set fn10 jx
    set fn11 jy
    
    Form2d .e "Enter page setup parameters. mar denotes the fractional page area on each side to use as a margin (0 to 0.5). jx and jy are the fractional justification relative to the center (-0.5 to 0.5). aspect is the page aspect ratio (0 preserves original aspect ratio)."
    tkwait window .e
    
    [plwin] page $fv00 $fv01 $fv10 $fv11
}

#----------------------------------------------------------------------------
# page_reset
#
# Resets page parameters.
#----------------------------------------------------------------------------

itcl::body Plplotwin::page_reset {} {
    [plwin] page 0. 0. 0. 0.
}

#----------------------------------------------------------------------------
# zoom_start
#
# Starts plot zoom.
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_start {wx wy} {
    global def_button_cmd
    
    bind [plwin] <ButtonPress> $def_button_cmd
    label_set "Select zoom region by dragging mouse, then release."
    
    [plwin] draw init
    bind [plwin] <B1-Motion> [itcl::code $this zoom_mouse_draw $wx $wy %x %y]
    bind [plwin] <B1-ButtonRelease> [itcl::code $this zoom_mouse_end $wx $wy %x %y]
}

#----------------------------------------------------------------------------
# zoom_coords
#
# Transforms the initial and final mouse coordinates to either:
#
# opt = 0 device coordinates
# opt = 1 normalized device coordinates
#
# -zoompreservesaspect
# 0 box follows mouse movements exactly
# 1 box follows mouse movements so that aspect ratio is preserved (default)
#
# -zoomfromcenter
# 0 first and last points specified determine opposite corners
# of zoom box.
# 1 box is centered about the first point clicked on, 
# perimeter follows mouse (default)
#
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_coords {x0 y0 x1 y1 opt} {
    set Lx [winfo width [plwin]]
    set Ly [winfo height [plwin]]
    
    # Enforce boundaries in device coordinate space
    
    set bounds [[plwin] view bounds]
    set xmin [expr [lindex "$bounds" 0] * $Lx]
    set ymin [expr [lindex "$bounds" 1] * $Ly]
    set xmax [expr [lindex "$bounds" 2] * $Lx]
    set ymax [expr [lindex "$bounds" 3] * $Ly]
    
    set x1 [max $xmin [min $xmax $x1]]
    set y1 [max $ymin [min $ymax $y1]]
    
    # Two-corners zoom.
    
    if { $itk_option(-zoomfromcenter) == 0 } {
	
	# Get box lengths
	
	set dx [expr $x1 - $x0]
	set dy [expr $y1 - $y0]
	
	set sign_dx [expr ($dx > 0) ? 1 : -1]
	set sign_dy [expr ($dy > 0) ? 1 : -1]
	
	set xl $x0
	set yl $y0
	
	# Constant aspect ratio
	
	if { $itk_option(-zoompreservesaspect)} {
	    
	    # Scale factors used to maintain plot aspect ratio
	    
	    set xscale [expr $xmax - $xmin]
	    set yscale [expr $ymax - $ymin]
	    
	    # Adjust box size for proper aspect ratio
	    
	    set rx [expr double(abs($dx)) / $xscale]
	    set ry [expr double(abs($dy)) / $yscale]
	    
	    if { $rx > $ry } {
		set dy [expr $yscale * $rx * $sign_dy]
	    } else {
		set dx [expr $xscale * $ry * $sign_dx]
	    }
	    
	    set xr [expr $xl + $dx]
	    set yr [expr $yl + $dy]
	    
	    # Now check again to see if in bounds, and adjust if not
	    
	    if { $xr < $xmin || $xr > $xmax } {
		if { $xr < $xmin } {
		    set dx [expr $xmin - $x0]
		} else {
		    set dx [expr $xmax - $x0]
		}
		set rx [expr double(abs($dx)) / $xscale]
		set dy [expr $yscale * $rx * $sign_dy]
	    }
	    
	    if { $yr < $ymin || $yr > $ymax } {
		if { $yr < $ymin } {
		    set dy [expr $ymin - $y0]
		} else {
		    set dy [expr $ymax - $y0]
		}
		set ry [expr double(abs($dy)) / $yscale]
		set dx [expr $xscale * $ry * $sign_dx]
	    }
	}
	
	# Final box coordinates
	
	set xr [expr $xl + $dx]
	set yr [expr $yl + $dy]
	
	# zoom from center out, preserving aspect ratio
	
    } else {
	
	# Get box lengths, adjusting downward if necessary to keep in bounds
	
	set dx [expr abs($x1 - $x0)]
	set dy [expr abs($y1 - $y0)]
	
	set xr [expr $x0 + $dx]
	set xl [expr $x0 - $dx]
	set yr [expr $y0 + $dy]
	set yl [expr $y0 - $dy]
	
	if { $xl < $xmin } {
	    set dx [expr {$x0 - $xmin}]
	}
	if { $xr > $xmax } {
	    set dx [expr {$xmax - $x0}]
	}
	if { $yl < $ymin } {
	    set dy [expr {$y0 - $ymin}]
	}
	if { $yr > $ymax } {
	    set dy [expr {$ymax - $y0}]
	}
	
	# Constant aspect ratio
	
	if { $itk_option(-zoompreservesaspect) } {
	    
	    # Scale factors used to maintain plot aspect ratio
	    
	    set xscale [expr {$xmax - $xmin}]
	    set yscale [expr {$ymax - $ymin}]
	    
	    # Adjust box size for proper aspect ratio
	    
	    set rx [expr {double($dx) / $xscale}]
	    set ry [expr {double($dy) / $yscale}]
	    if { $rx > $ry } {
		set dy [expr {$yscale * $rx}]
	    } else {
		set dx [expr {$xscale * $ry}]
	    }
	    
	    set xr [expr $x0 + $dx]
	    set xl [expr $x0 - $dx]
	    set yr [expr $y0 + $dy]
	    set yl [expr $y0 - $dy]
	    
	    # Now check again to see if in bounds, and adjust downward if not
	    
	    if { $xl < $xmin } {
		set dx [expr $x0 - $xmin]
		set rx [expr double($dx) / $xscale]
		set dy [expr $yscale * $rx]
	    }
	    if { $xr > $xmax } {
		set dx [expr $xmax - $x0]
		set rx [expr double($dx) / $xscale]
		set dy [expr $yscale * $rx]
	    }
	    if { $yl < $ymin } {
		set dy [expr $y0 - $ymin]
		set ry [expr double($dy) / $yscale]
		set dx [expr $xscale * $ry]
	    }
	    if { $yr > $ymax } {
		set dy [expr $ymax - $y0]
		set ry [expr double($dy) / $yscale]
		set dx [expr $xscale * $ry]
	    }
	}
	
	# Final box coordinates
	
	set xr [expr {$x0 + $dx}]
	set xl [expr {$x0 - $dx}]
	set yr [expr {$y0 + $dy}]
	set yl [expr {$y0 - $dy}]
    }
    
    # Optional translation to relative device coordinates.
    
    if { $opt == 1 } {
	set wxl [expr {$xl / double($Lx)} ]
	set wxr [expr {$xr / double($Lx)} ]
	set wyl [expr {1.0 - $yr / double($Ly)} ]
	set wyr [expr {1.0 - $yl / double($Ly)} ]
	
    } else {
	# These 'int' are required, because if we have a very low
	# tcl_precision, our numbers can be turned to floating point
	# stuff like 1.2e2 which causes major errors in plwin draw rect.
	set wxl [expr {int($xl)}]
	set wxr [expr {int($xr)}]
	set wyl [expr {int($yl)}]
	set wyr [expr {int($yr)}]
    }
    return [list $wxl $wyl $wxr $wyr]
}

#----------------------------------------------------------------------------
# zoom_mouse_draw
#
# Draws zoom box in response to mouse motion (with button held down).
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_mouse_draw {wx0 wy0 wx1 wy1} {
    eval [plwin] draw rect [zoom_coords $wx0 $wy0 $wx1 $wy1 0]
}

#----------------------------------------------------------------------------
# zoom_mouse_end
#
# Performs actual zoom, invoked when user releases mouse button.
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_mouse_end {wx0 wy0 wx1 wy1} {
    
    # Finish rubber band draw
    
    bind [plwin] <B1-ButtonRelease> {}
    bind [plwin] <B1-Motion> {}
    label_reset 
    [plwin] draw end
    
    # Select new plot region
    
    eval view_zoom [zoom_coords $wx0 $wy0 $wx1 $wy1 1]
}

#----------------------------------------------------------------------------
# view_select
#
# Handles change of view into plot.
# Given in relative plot window coordinates.
#----------------------------------------------------------------------------

itcl::body Plplotwin::view_select {x0 y0 x1 y1} {
    
    # Adjust arguments to be in bounds and properly ordered (xl < xr, etc)
    
    set xl [min $x0 $x1]
    set yl [min $y0 $y1]
    set xr [max $x0 $x1]
    set yr [max $y0 $y1]
    
    set xmin 0.
    set ymin 0.
    set xmax 1.
    set ymax 1.
    
    set xl [max $xmin [min $xmax $xl]]
    set yl [max $ymin [min $ymax $yl]]
    set xr [max $xmin [min $xmax $xr]]
    set yr [max $ymin [min $ymax $yr]]
    
    # Only create scrollbars if really needed.
    
    if {($xl == $xmin) && ($xr == $xmax)} {set hscroll 0} else {set hscroll 1}
    
    if {($yl == $xmin) && ($yr == $xmax)} {set vscroll 0} else {set vscroll 1}
    
    if { ! ($hscroll || $vscroll)} {return}
    
    # Select plot region
    
    [plwin] view select $xl $yl $xr $yr
    
    # Fix up view
    
    fixview $hscroll $vscroll
}

#----------------------------------------------------------------------------
# view_zoom
#
# Handles zoom.
# Given in relative device coordinates.
#----------------------------------------------------------------------------

itcl::body Plplotwin::view_zoom {x0 y0 x1 y1} {
    global xl xr yl yr
    
    # Adjust arguments to be properly ordered (xl < xr, etc)
    
    set xl [min $x0 $x1]
    set yl [min $y0 $y1]
    set xr [max $x0 $x1]
    set yr [max $y0 $y1]
    
    # Check for double-click (specified zoom region less than a few pixels
    # wide). In this case, magnification is 2X in each direction, centered at
    # the mouse location. At the boundary, the magnification is determined
    # by the distance to the boundary.
    
    set stdzoom 0.5
    if { ($xr - $xl < 0.02) && ($yr - $yl < 0.02) } {
	set nxl [expr $xl - 0.5 * $stdzoom]
	set nxr [expr $xl + 0.5 * $stdzoom]
	if { $nxl < 0.0 } {
	    set nxl 0.0
	    set nxr [expr 2.0 * $xl]
	} 
	if { $nxr > 1.0 } {
	    set nxr 1.0
	    set nxl [expr 2.0 * $xl - 1.0]
	}
	set xl $nxl
	set xr $nxr
	
	set nyl [expr $yl - 0.5 * $stdzoom]
	set nyr [expr $yl + 0.5 * $stdzoom]
	if { $nyl < 0.0 } {
	    set nyl 0.0
	    set nyr [expr 2.0 * $yl]
	}
	if { $nyr > 1.0 } {
	    set nyr 1.0
	    set nyl [expr 2.0 * $yl - 1.0]
	}
	set yl $nyl
	set yr $nyr
    }
    
    # Adjust arguments to be in bounds (in case margins are in effect).
    
    set bounds [[plwin] view bounds]
    foreach {xmin ymin xmax ymax} $bounds {}
    
    set xl [max $xmin [min $xmax $xl]]
    set yl [max $ymin [min $ymax $yl]]
    set xr [max $xmin [min $xmax $xr]]
    set yr [max $ymin [min $ymax $yr]]
    
    # Only create scrollbars if really needed.
    
    set hscroll [expr {($xl != $xmin) || ($xr != $xmax)}]
    set vscroll [expr {($yl != $ymin) || ($yr != $ymax)}]
    
    if {!($hscroll || $vscroll)} {
	[plwin] redraw
	return
    }
    
    # Select plot region
    
    [plwin] view zoom $xl $yl $xr $yr wait
    
    # Fix up view
    
    fixview $hscroll $vscroll
    
    # Add window to zoom windows list
    
    incr zidx
    set zidx_max $zidx
    
    set zcoords($zidx) [[plwin] view]
}

#----------------------------------------------------------------------------
# zoom_back
#
# Traverses the zoom windows list backward.
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_back {} {
    if { $zidx == 0 } then return
    
    incr zidx -1
    
    # Select plot region
    
    eval [plwin] view select $zcoords($zidx)
}

#----------------------------------------------------------------------------
# zoom_forward
#
# Traverses the zoom windows list forward.
#----------------------------------------------------------------------------

itcl::body Plplotwin::zoom_forward {} {
    if { $zidx_max == 0 || $zidx == $zidx_max } {return}
    
    incr zidx
    
    # Select plot region
    
    eval [plwin] view select $zcoords($zidx)
}

#----------------------------------------------------------------------------
# view_scroll
#
# Scrolls view incrementally.
# Similar to clicking on arrow at end of scrollbar (but speed is user
# controllable).
#----------------------------------------------------------------------------

itcl::body Plplotwin::view_scroll {dx dy s} {
    global key_scroll_mag
    global key_scroll_speed
    
    # Set up multiplication factor
    
    set mult $key_scroll_speed
    if { $s & 0x01 } {
	set mult [expr $mult * $key_scroll_mag]
    }
    if { $s & 0x02 } {
	set mult [expr $mult * $key_scroll_mag]
    }
    if { $s & 0x04 } {
	set mult [expr $mult * $key_scroll_mag]
    }
    if { $s & 0x08 } {
	set mult [expr $mult * $key_scroll_mag]
    }
    
    # Now scroll
    
    if {($dx != 0) && \
      [winfo exists $itk_interior.hscroll] && [winfo ismapped $itk_interior.hscroll] } then {
	
	set dx [expr $dx * $mult]
	set first [lindex [$itk_interior.hscroll get] 2]
	[plwin] xview scroll [expr $first+$dx] units
    }
    if {($dy != 0) && \
      [winfo exists $itk_interior.vscroll] && [winfo ismapped $itk_interior.vscroll] } then {
	
	set dy [expr $dy * $mult]
	set first [lindex [$itk_interior.vscroll get] 2]
	[plwin] yview scroll [expr $first+$dy] units
    }
}

#----------------------------------------------------------------------------
# fixview
#
# Handles updates of scrollbars & plot after view change.
#----------------------------------------------------------------------------

itcl::body Plplotwin::fixview {hscroll vscroll} {
    
    # Create scrollbars if they don't already exist.
    
    if { $hscroll && ! [winfo exists $itk_interior.hscroll] } {
	scrollbar $itk_interior.hscroll -relief sunken -orient horiz \
	  -command "[plwin] xview"
	[plwin] config -xscroll "$itk_interior.hscroll set"
    }
    if { $vscroll && ! [winfo exists $itk_interior.vscroll] } {
	scrollbar $itk_interior.vscroll -relief sunken \
	  -command "[plwin] yview"
	[plwin] config -yscroll "$itk_interior.vscroll set"
    }
    
    # Map scrollbars if not already mapped.
    
    if { ($hscroll && ! [winfo ismapped $itk_interior.hscroll]) || \
      ($vscroll && ! [winfo ismapped $itk_interior.vscroll]) } {
	#update
	
	if { $hscroll } {
	    grid $itk_interior.hscroll -column 0 -row 2 -sticky swe
	}
	if { $vscroll } {
	    grid $itk_interior.vscroll -sticky nse -column 1 -row 1
	}
    } else {
	[plwin] redraw
    }
}

#----------------------------------------------------------------------------
# update_view
#
# Updates view. Results in scrollbars being added if they are appropriate.
# Does nothing if the plot window is unchanged from the default.
#----------------------------------------------------------------------------

itcl::body Plplotwin::update_view {} {
    eval view_select [[plwin] view]
}

#----------------------------------------------------------------------------
# status_msg
#
# Used for temporarily flashing a status message in the status bar. Better
# than a dialog because it can be ignored and will go away on its own.
#----------------------------------------------------------------------------

itcl::body Plplotwin::status_msg {msg} {
    label_set $msg
    after 2500 [itcl::code $this label_reset]
}

#----------------------------------------------------------------------------
# label_reset
#
# Resets message in status bar to the default.
#----------------------------------------------------------------------------

itcl::body Plplotwin::label_reset {} {
    $itk_component(lstat) configure -text ""
}

#----------------------------------------------------------------------------
# label_set
#
# Sets message in status bar.
# 
# EXTREMELY IMPORTANT.  This procedure must not re-enter
# Tcl's event loop.  You must not call 'update', 'update idletasks'
# or anything like that.  A future version of the plframe may avoid
# this problem, but right now it can cause horrible problems when
# you have two or more plframes both of which need to be updated
# simultaneously.  If you have 'update idletasks', the when the
# plframe calls this method (in the middle of updating itself),
# Tcl will give the other frame a chance to update too.  This 
# leads to horrible confusion, involving one window drawing all
# over the other.
#----------------------------------------------------------------------------

itcl::body Plplotwin::label_set {msg} {
    # It's a single line message, so get rid of newlines
    regsub -all "\[\r\n\]" $msg " "  msg
    $itk_component(lstat) configure -text " $msg"
}

#----------------------------------------------------------------------------
# dplink
#
# Initializes socket data link between widget and client code.
# In addition, as this is the last client/server connection needed, I
# disable further connections.
#----------------------------------------------------------------------------

itcl::body Plplotwin::dplink {client} {
    
    global list_sock data_sock
    
    dp_Host +
    set rv [dp_connect -server 0]
    set list_sock [lindex $rv 0]
    set data_port [lindex $rv 1]
    
    dp_RDO $client set data_port $data_port
    set data_sock [lindex [dp_accept $list_sock] 0]
    [plwin] openlink socket $data_sock
    dp_Host -
}














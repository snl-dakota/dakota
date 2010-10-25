#----------------------------------------------------------------------------
# $Id: x11.tcl 3186 2006-02-15 18:17:33Z slbrow $
#----------------------------------------------------------------------------

# Does a series of mesh plots for a given data set, with different
# viewing options in each plot.

# Routine for restoring colour map1 to default.
# See static void plcmap1_def(void) in plctrl.c for reference.
proc restore_cmap1_11 {w} {
   # For center control points, pick black or white, whichever is closer to bg 
   # Be careful to pick just short of top or bottom else hue info is lost
   $w cmd plgcolbg rbg gbg bbg
   set vertex [expr ($rbg + $gbg + $bbg)/(3.*255.)]
   if {$vertex < 0.5} {
      set vertex 0.01
      set midpt 0.10
   } else {
      set vertex 0.99
      set midpt 0.90
   }
   # Independent variable of control points.
   matrix i f 6 = {0., 0.44, 0.50, 0.50, 0.56, 1.}
   # Hue for control points.  Blue-violet to red
   matrix h f 6 = {260., 260., 260., 0., 0., 0.}
   # Lightness ranging from medium to vertex to medium
   # double quotes required rather than braces so that variables
   # get evaluated for initialization.
   matrix l f 6  = "0.5, $midpt, $vertex, $vertex, $midpt, 0.5"
   # Saturation is complete for default
   matrix s f 6 = {1., 1., 1., 1., 1., 1.}
   # Integer flag array is zero (no interpolation along far-side of colour
   # figure
   matrix rev i 6 = {0, 0, 0, 0, 0, 0}
   # Default number of cmap1 colours
   $w cmd plscmap1n 128
   # Interpolate between control points to set up default cmap1.
   $w cmd plscmap1l 0 6 i h l s rev
}

# Routine for initializing color map 1 in HLS space.
# Basic grayscale variation from half-dark (which makes more interesting
# looking plot compared to dark) to light.
proc cmap1_init_11 {w gray} {
   # Independent variable of control points.
   matrix i f 2 = {0., 1.}
   if {$gray == 1} {
      # Hue for control points.  Doesn't matter since saturation is zero.
      matrix h f 2 = {0., 0.}
      # Lightness ranging from half-dark (for interest) to light.
      matrix l f 2 = {0.5, 1.}
      # Gray scale has zero saturation.
      matrix s f 2 = {0., 0.}
   } else {
      # Hue ranges from blue (240 deg) to red (0 or 360 deg)
      matrix h f 2 = {240., 0.}
      # Lightness and saturation are constant (values taken from C example).
      matrix l f 2 = {0.6, 0.6}
      matrix s f 2 = {0.8, 0.8}
   }

   # Integer flag array is zero (no interpolation along far-side of colour
   # figure
   matrix rev i 2 = {0, 0}
   # Number of cmap1 colours is 256 in this case. 
   $w cmd plscmap1n 256
   # Interpolate between control points to set up default cmap1.
   $w cmd plscmap1l 0 2 i h l s rev
}

proc x11 {{w loopback}} {

    # these should be defined elsewhere.
    # Must use numerical rather than hex value for this one since used
    # inside an array
#    set DRAW_LINEXY 0x03
    set DRAW_LINEXY 3
    set MAG_COLOR 0x04
    set BASE_CONT 0x08
    matrix opt i 4 = "$DRAW_LINEXY, $DRAW_LINEXY"
    matrix alt f 4 = {33.0, 17.0}
    matrix az  f 4 = {24.0, 115.0}

    set xpts 35
    set ypts 46

    matrix x f $xpts
    matrix y f $ypts
    matrix z f $xpts $ypts

    for {set i 0} {$i < $xpts} {incr i} {
	x $i = [expr 3.* ($i - ($xpts/2)) / double($xpts/2) ]
    }

    for {set i 0} {$i < $ypts} {incr i} {
	y $i = [expr 3.* ($i - ($ypts/2)) / double($ypts/2) ]
    }

    for {set i 0} {$i < $xpts} {incr i} {
	set xx [x $i]
	for {set j 0} {$j < $ypts} {incr j} {
	    set yy [y $j]
	    z $i $j = [expr 3. * (1.-$xx)*(1.-$xx) * exp(-($xx*$xx) - \
	      ($yy+1.)*($yy+1.)) - 10. * ($xx/5. - pow($xx,3.) - \
	      pow($yy,5.)) * exp(-$xx*$xx-$yy*$yy) - \
	      1./3. * exp(-($xx+1)*($xx+1) - ($yy*$yy))]
	    # Jungfraujoch/Interlaken 
	    if {1==2} {
	      set zz [z $i $j]
	      if {$zz <= -1.} {
		z $i $j = [expr -1]
	      }
	    }
	}
    }

    set zmin [z min [ expr $xpts * $ypts]]
    set zmax [z max [ expr $xpts * $ypts]]

    set nlev 10
    matrix clev f $nlev
    set step [expr {($zmax-$zmin)/($nlev+1)}]
    for {set i 0} {$i < $nlev} {incr i} {
	clev $i = [expr {$zmin + ($i+1) * $step}]
    }

    cmap1_init_11 $w 0
    for {set k 0} {$k < 2} {incr k} {
       for {set i 0} {$i < 4} {incr i} {
	$w cmd pladv 0
	$w cmd plcol0 1
	$w cmd plvpor 0.0 1.0 0.0 0.9
	$w cmd plwind -1.0 1.0 -1.0 1.5

	$w cmd plw3d 1.0 1.0 1.2 -3.0 3.0 -3.0 3.0 $zmin $zmax [alt $k] [az $k]
	$w cmd plbox3 "bnstu" "x axis" 0.0 0 \
	    "bnstu" "y axis" 0.0 0 \
	    "bcdmnstuv" "z axis" 0.0 4

	$w cmd plcol0 2

        # wireframe plot
	if {$i == 0} {
	   $w cmd plmesh x y z [expr [opt $k]]

        # magnitude colored wireframe plot
        } elseif {$i == 1} {
	   $w cmd plmesh x y z [expr [opt $k] | $MAG_COLOR]

        # magnitude colored wireframe plot with sides
        } elseif {$i == 2} {
	   $w cmd plot3d x y z [expr [opt $k] | $MAG_COLOR] 1

        # magnitude colored wireframe plot with base contour
        } elseif {$i == 3} {
	   $w cmd plmeshc x y z $xpts $ypts \
	     [expr [opt $k] | $MAG_COLOR | $BASE_CONT] clev $nlev
        }

	$w cmd plcol0 3

	set title [format "#frPLplot Example 11 - Alt=%.0f, Az=%.0f, Opt=%d" \
		       [alt $k] [az $k] [opt $k] ]
	$w cmd plmtex "t" 1.0 0.5 0.5 $title
       }
    }
# Restore defaults
    $w cmd plcol0 1
    restore_cmap1_11 $w
}

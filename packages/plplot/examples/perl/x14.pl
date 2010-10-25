#! /usr/bin/env perl
#
# Demo x14 for the PLplot PDL binding
#
# Demo of multiple stream/window capability (requires Tk or Tcl-DP).
#
# Copyright (C) 2004  Rafael Laboissiere
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Library Public License as published
# by the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

# SYNC: x14c.c 1.26

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

my ($xscale, $yscale, $xoff, $yoff, $xs, $ys);
my $space1 = 1500; my $mark1 = 1500;

#--------------------------------------------------------------------------
# main
#
# Plots several simple functions from other example programs.
#
# This version sends the output of the first 4 plots (one page) to two
# independent streams.
#--------------------------------------------------------------------------

sub main {
  my $xleng0 = 400; my $yleng0 = 300; my $xoff0 = 200; my $yoff0 = 200;
  my $xleng1 = 400; my $yleng1 = 300; my $xoff1 = 500; my $yoff1 = 500;

  # Select either TK or DP driver and use a small window
  # Using DP results in a crash at the end due to some odd cleanup problems
  # The geometry strings MUST be in writable memory

  $geometry_master = "500x410+100+200";
  $geometry_slave  = "500x410+650+200";

  # plplot initialization
  # Parse and process command line arguments

  plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

  my $driver = plgdev ();

  print ("Demo of multiple output streams via the $driver driver.\n"
         . "Running with the second stream as slave to the first.\n"
         . "\n");

  # Set up first stream

  plsetopt ("geometry", $geometry_master);

  plsdev ($driver);
  plssub (2, 2);
  plinit ();

  # Start next stream

  plsstrm (1);

  # Turn off pause to make this a slave (must follow master)

  plsetopt ("geometry", $geometry_slave);
  plspause (0);
  plsdev ($driver);
  plinit ();

  # Set up the data & plot
  # Original case

  plsstrm (0);

  $xscale = 6.;
  $yscale = 1.;
  $xoff = 0.;
  $yoff = 0.;
  plot1 ();

  # Set up the data & plot

  $xscale = 1.;
  $yscale = 1.e+6;
  plot1 ();

  # Set up the data & plot

  $xscale = 1.;
  $yscale = 1.e-6;
  $digmax = 2;
  plsyax ($digmax, 0);
  plot1 ();

  # Set up the data & plot

  $xscale = 1.;
  $yscale = 0.0014;
  $yoff = 0.0185;
  $digmax = 5;
  plsyax ($digmax, 0);
  plot1 ();

  # To slave
  # The pleop() ensures the eop indicator gets lit.

  plsstrm (1);
  plot4 ();
  pleop ();

  # Back to master

  plsstrm (0);
  plot2 ();
  plot3 ();

  # To slave

  plsstrm (1);
  plot5 ();
  pleop ();

  # Back to master to wait for user to advance

  plsstrm (0);
  pleop ();

  # Call plend to finish off.

  plend ();
}

# ===============================================================

sub plot1 {
  my $x = $xoff + $xscale * (sequence (60) + 1) / 60;
  my $y = $yoff + $yscale * $x ** 2;

  my $xmin = $x->index (0);
  my $xmax = $x->index (59);
  my $ymin = $y->index (0);
  my $ymax = $y->index (59);

  my $i = sequence (6);
  my $xs = $x->index ($i * 10 + 3);
  my $ys = $y->index ($i * 10 + 3);


  # Set up the viewport and window using PLENV. The range in X is
  # 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
  # scaled separately (just = 0), and we just draw a labelled
  # box (axis = 0).

  plcol0 (1);
  plenv ($xmin, $xmax, $ymin, $ymax, 0, 0);
  plcol0 (6);
  pllab ("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

  # Plot the data points

  plcol0 (9);
  plpoin ($xs, $ys, 9);

  # Draw the line through the data

  plcol0 (4);
  plline ($x, $y);
  plflush ();
}

# ===============================================================

sub plot2 {

  # Set up the viewport and window using PLENV. The range in X is -2.0 to
  # 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
  # ($just = 0), and we draw a box with axes (axis = 1).

  plcol0 (1);
  plenv (-2.0, 10.0, -0.4, 1.2, 0, 1);
  plcol0 (2);
  pllab ("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

  # Fill up the arrays

  my $i = sequence (100);

  my $x = ($i - 19.0) / 6.0;
  my $y = ones (100);
  my $idx = which ($x);
  $y->index ($idx) .= sin ($x->index ($idx)) / $x->index ($idx);

  # Draw the line

  plcol0 (3);
  plline ($x, $y);
  plflush ();
}

# ===============================================================

sub plot3 {
  # For the final graph we wish to override the default tick intervals, and
  # so do not use PLENV

  pladv (0);

  # Use standard viewport, and define X range from 0 to 360 degrees, Y range
  # from -1.2 to 1.2.

  plvsta ();
  plwind (0.0, 360.0, -1.2, 1.2);

  # Draw a box with ticks spaced 60 degrees apart in X, and 0.2 in Y.

  plcol0 (1);
  plbox (60.0, 2, 0.2, 2, "bcnst", "bcnstv");

  # Superimpose a dashed line grid, with 1.5 mm marks and spaces.

  plstyl ($mark1, $space1);
  plcol0 (2);
  plbox (30.0, 0, 0.2, 0, "g", "g");
  plstyl (pdl ([]), pdl ([]));

  plcol0 (3);
  pllab ("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

  my $i = sequence (101);
  my $x = 3.6 * $i;
  my $y = sin ($x * pi / 180);

  plcol0 (4);
  plline ($x, $y);
  plflush ();
}

# ===============================================================

sub plot4 {
  my $dtr = pi / 180;
  my $i = sequence (361);
  my $x0 = cos ($dtr * $i);
  my $y0 = sin ($dtr * $i);

  # Set up viewport and window, but do not draw box

  plenv (-1.3, 1.3, -1.3, 1.3, 1, -2);
  for (my $i = 1; $i <= 10; $i++) {
    my $j = sequence (361);
    my $x = 0.1 * $i * $x0;
    my $y = 0.1 * $i * $y0;

    # Draw circles for polar grid

    plline($x, $y);
  }

  plcol0 (2);
  for (my $i = 1; $i <= 11; $i++) {
    my $theta = 30 * $i;
    my $dx = cos ($dtr * $theta);
    my $dy = sin ($dtr * $theta);

    # Draw radial spokes for polar grid

    pljoin (0.0, 0.0, $dx, $dy);
    my $text = sprintf ("%.0f", $theta);

    # Write labels for angle

    # Slightly off zero to avoid floating point logic flips at 90 and 270 deg.
    if ($dx >= -0.00001) {
      plptex ($dx, $dy, $dx, $dy, -0.15, $text);
    } else {
      plptex($dx, $dy, -$dx, -$dy, 1.15, $text);
    }
  }

  # Draw the graph

  $i = sequence (361);
  my $r = sin ($dtr * (5 * $i));
  $x = $x0 * $r;
  $y = $y0 * $r;

  plcol0 (3);
  plline ($x, $y);

  plcol0 (4);
  plmtex (2.0, 0.5, 0.5,
         "t", "#frPLplot Example 3 - r(#gh)=sin 5#gh");
  plflush ();
}

# ===============================================================

# Demonstration of contour plotting

use constant XPTS => 35;
use constant YPTS => 46;
use constant XSPA => 2 / (XPTS - 1);
use constant YSPA => 2 / (YPTS - 1);

my @tr = (XSPA, 0.0, -1.0, 0.0, YSPA, -1.0);

sub mypltr {
  my ($x, $y) = @_;
  my $tx = $tr[0] * $x + $tr[1] * $y + $tr[2];
  my $ty = $tr[3] * $x + $tr[4] * $y + $tr[5];
  return ($tx, $ty);
}

my $ clevel = pdl [-1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1.];

sub plot5 {

  my $mark = 1500; my $space = 1500;

  my $xx = ((sequence (XPTS) - (XPTS / 2)) / (XPTS / 2))->dummy (1, YPTS);
  my $yy = ((sequence (YPTS) - (YPTS / 2))
            / (YPTS / 2) - 1.0)->dummy (0, XPTS);
  my $z = $xx * $xx - $yy * $yy;
  my $w = 2 * $xx * $yy;

  plenv (-1.0, 1.0, -1.0, 1.0, 0, 0);
  plcol0 (2);
  plcont ($z, 1, XPTS, 1, YPTS, $clevel, \&mypltr, 0);
  plstyl ($mark, $space);
  plcol0 (3);
  plcont ($w, 1, XPTS, 1, YPTS, $clevel, \&mypltr, 0);
  plcol0 (1);
  pllab ("X Coordinate", "Y Coordinate", "Streamlines of flow");
  plflush ();
}


main ();

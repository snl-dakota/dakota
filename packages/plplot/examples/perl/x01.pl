#! /usr/bin/env perl
#
# Demo x01 for the PLplot PDL binding
#
# Simple line plot and multiple windows demo
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

# SYNC: x01c.c 1.39

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];
use Time::HiRes qw [usleep];
use POSIX qw [isprint];
use Getopt::Long qw [:config pass_through];
use Text::Wrap;

$Text::Wrap::columns = 72;

$xscale = 6.;
$yscale = 1.;
$xoff = 0.;
$yoff = 0.;

my $test_xor = 0;

#--------------------------------------------------------------------------
# main
#
# Generates several simple line plots.  Demonstrates:
#   - subwindow capability
#   - setting up the window, drawing plot, and labelling
#   - changing the color
#   - automatic axis rescaling to exponential notation
#   - placing the axes in the middle of the box
#   - gridded coordinate axes
#--------------------------------------------------------------------------

sub main {

  # Options data structure definition. */

  my $locate_mode = 0;
  my $fontset = 1;

  GetOptions ("locate" => \$locate_mode,
              "xor"    => \$test_xor,
              "font=i" => \$fontset,
              "save=s" => \$f_name,
              "help"   => \$help);

  my @notes = ("Make sure you get it right!");

  if ($help) {
    print (<<EOT);
$0 options:
    --locate              Turns on test of API locate function
    --xor                 Turns on test of XOR
    --font number         Selects stroke font set (0 or 1, def:1)
    --save filename       Save plot in color postscript 'filename'

EOT
    print (wrap ('', '', @notes), "\n");
    push (@ARGV, "-h");
  }

  unshift (@ARGV, $0);

  # plplot initialization
  # Divide page into 2x2 plots unless user overrides

  plssub (2, 2);

  # Parse and process command line arguments

  plParseOpts (\@ARGV, PL_PARSE_PARTIAL);

  # Get version number, just for kicks */

  my $ver = plgver ();
  print STDERR "PLplot library version: $ver\n";

  # Initialize plplot

  plinit;

  # Select font set as per input flag

  plfontld ($fontset);

  # Set up the data
  # Original case

  $xscale = 6.;
  $yscale = 1.;
  $xoff = 0.;
  $yoff = 0.;

  # Do a plot

  plot1 (0);

  # Set up the data

  $xscale = 1.;
  $yscale = 0.0014;
  $yoff = 0.0185;

  $digmax = 5;
  plsyax ($digmax, 0);

  plot1 (1);

  plot2 ();

  plot3 ();

  #
  # Show how to save a plot:
  # Open a new device, make it current, copy parameters,
  # and replay the plot buffer
  #

  if ($f_name) { # command line option '--save filename'

    print (<<"EOT");
The current plot was saved in color Postscript under the name `$f_name'
EOT

    my $cur_strm = plgstrm ();    # get current stream
    my $new_strm = plmkstrm ();   # create a new one

    plsfnam ($f_name);            # file name
    plsdev ("psc");               # device type

    plcpstrm ($cur_strm, 0);      # copy old stream parameters to new stream
    plreplot ();                  # do the save by replaying the plot buffer
    plend1 ();                    # finish the device

    plsstrm ($cur_strm);          # return to previous stream
  }

  # Let's get some user input

  if ($locate_mode) {
    while (1) {
      my %gin = plGetCursor ();
      my $k = $gin{keysym};
      last if not %gin or $k == PLK_Escape;

      pltext ();

      printf ("subwin = $gin{subwindow}, wx = %f,  wy = %f, dx = %f,  "
              . "dy = %f,  c = "
              . ($k < 0xFF and isprint (chr $k) ? "'%c'" : "0x%02x")
              . "\n", $gin{wX}, $gin{wY}, $gin{dX}, $gin{dY}, $k);

      plgra ();
    }
  }

  # Don't forget to call plend() to finish off!

  plend;

}

sub plot1 {

  my $do_test = shift;

  my $x = $xoff + $xscale * (1 + sequence (60)) / 60.0;
  my $y = $yoff + $yscale * ($x ** 2);

  $xmin = $x->index (0);
  $xmax = $x->index (59);
  $ymin = $y->index (0);
  $ymax = $y->index (59);

  my $idx = sequence (6) * 10 + 3;
  $xs = $x->index ($idx);
  $ys = $y->index ($idx);

  # Set up the viewport and window using PLENV. The range in X is
  # 0.0 to 6.0, and the range in Y is 0.0 to 30.0. The axes are
  # scaled separately (just = 0), and we just draw a labelled
  # box (axis = 0).

  plcol0 (1);
  plenv ($xmin, $xmax, $ymin, $ymax, 0, 0);
  plcol0 (2);
  pllab ("(x)", "(y)", "#frPLplot Example 1 - y=x#u2");

  # Plot the data points

  plcol0 (4);
  plpoin ($xs, $ys, 9);

  # Draw the line through the data

  plcol0 (3);
  plline ($x, $y);

# xor mode enable erasing a line/point/text by replotting it again
# it does not work in double buffering mode, however

  if ($do_test and $test_xor) {
    my $st = plxormod (1);     # enter xor mode
    if ($st) {
      for (my $i = 0; $i < 60; $i++) {
        my $xi = $x->index ($i);
        my $yi = $y->index ($i);
        plpoin ($xi, $yi, 9);  # draw a point
        usleep (50000);	       # wait a little
        plflush ();            # force an update of the tk driver
        plpoin ($xi, $yi, 9);  # erase point
      }
      plxormod (0);            # leave xor mode
    }
  }
}

sub plot2 {

  # Set up the viewport and window using PLENV. The range in X is -2.0 to
  # 10.0, and the range in Y is -0.4 to 2.0. The axes are scaled separately
  # (just = 0), and we draw a box with axes (axis = 1).

  plcol0 (1);
  plenv (-2.0, 10.0, -0.4, 1.2, 0, 1);
  plcol0 (2);
  pllab ("(x)", "sin(x)/x", "#frPLplot Example 1 - Sinc Function");

  # Fill up the arrays

  my $x = (sequence (100) - 19.0) / 6.0;
  my $y = sin ($x) / $x;
  $y->index (which ($x == 0)) .= 1.0;

  # Draw the line

  plcol0 (3);
  plwid (2);
  plline ($x, $y);
  plwid (1);
}

sub plot3 {
    my $space1 = 1500;
    my $mark1 = 1500;

    # For the final graph we wish to override the default tick intervals, and
    # so do not use plenv().

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
    plstyl (pdl([]), pdl ([]));

    plcol0 (3);
    pllab ("Angle (degrees)", "sine", "#frPLplot Example 1 - Sine function");

    my $x = 3.6 * sequence (101);
    my $y = sin ($x * pi / 180.0);

    plcol0 (4);
    plline ($x, $y);
}

main ();

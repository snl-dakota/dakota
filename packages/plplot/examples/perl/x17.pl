#! /usr/bin/env perl
#
# Demo x17 for the PLplot PDL binding
#
# Plots a simple stripchart with four pens
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

# SYNC: x17c.c 1.16

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];
use Time::HiRes qw [usleep];

my $nsteps = 1000;

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# If db is used the plot is much more smooth. However, because of the
#  async X behaviour, one does not have a real-time scripcharter.

plsetopt ("db", "");
plsetopt ("np", "");

# User sets up plot completely except for window and data
# Eventually settings in place when strip chart is created will be
# remembered so that multiple strip charts can be used simultaneously.


# Specify some reasonable defaults for ymin and ymax
# The plot will grow automatically if needed (but not shrink)

my $ymin = -0.1;
my $ymax = 0.1;

# Specify initial tmin and tmax -- this determines length of window.
# Also specify maximum jump in t
# This can accomodate adaptive timesteps

my $tmin = 0.;
my $tmax = 10.;
my $tjump = 0.3;	# percentage of plot to jump

# Axes options same as plbox.
# Only automatic tick generation and label placement allowed
# Eventually I'll make this fancier

my $colbox = 1;
my $collab = 3;

# pens color and line style
my $styline = sequence (4) + 2;
my $colline = $styline;

# pens legend
my @legline = ("sum", "sin","sin*noi", "sin+noi");

# legend position
my $xlab = 0.;
my $ylab = 0.25;

my $autoy = 1;	# autoscale y
my $acc = 1;	# don't scrip, accumulate

# Initialize plplot

plinit ();

pladv (0);
plvsta ();

# Register our error variables with PLplot
# From here on, we're handling all errors here
# N.B : plsError is not implement in the PErlDL binding
# plsError (&errcode, errmsg);

my $id1 = plstripc ($tmin, $tmax, $tjump, $ymin, $ymax,
                    $xlab, $ylab,
                    $autoy, $acc,
                    $colbox, $collab,
                    $colline, $styline,
                    "bcnst", "bcnstv", \@legline,
                    "t", "", "Strip chart demo");

# N.B : plsError is not implement in the PErlDL binding
# if (errcode) {
#   fprintf(stderr, "%s\n", errmsg);
#   exit(1);
# }

# N.B : plsError is not implement in the PerlDL binding
# Let plplot handle errors from here on
# plsError(NULL, NULL);

$autoy = 0;     # autoscale y
$acc = 1;       # accumulate

# This is to represent a loop over time
# Let's try a random walk process

my $y1 = my $y2 = my $y3 = my $y4 = 0.0;
my $dt = 0.1;

for (my $n = 0; $n < $nsteps; $n++) {
  usleep (10000);    # wait a little (10 ms) to simulate time elapsing
  my $t = $n * $dt;
  $noise = rand (1.0) - 0.5;
  $y1 = $y1 + $noise;
  $y2 = sin ($t * pi / 18);
  $y3 = $y2 * $noise;
  $y4 = $y2 + $noise / 3;

  # There is no need for all pens to have the same number of
  # points or beeing equally time spaced.

  if ($n % 2) {
    plstripa ($id1, 0, $t, $y1);
  }
  if ($n % 3) {
    plstripa ($id1, 1, $t, $y2);
  }
  if ($n % 4) {
    plstripa ($id1, 2, $t, $y3);
  }
  if ($n % 5) {
    plstripa ($id1, 3, $t, $y4);
  }
  pleop ();  # use double buffer (-db on command line)
}

# Destroy strip chart and it's memory

plstripd ($id1);

plend ();

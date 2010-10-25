#! /usr/bin/env perl
#
# Demo x18 for the PLplot PDL binding
#
# 3-d line and point plot demo.
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

# SYNC: x18c.c 1.21

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

my @opt = (1, 0, 1, 0);
my @alt = (20.0, 35.0, 50.0, 65.0);
my @az = (30.0, 40.0, 50.0, 60.0);

sub test_poly {
  my $k = shift;

  my $draw = pdl ([ 1, 1, 1, 1 ],
                  [ 1, 0, 1, 0 ],
                  [ 0, 1, 0, 1 ],
                  [ 1, 1, 0, 0 ]);

  my $two_pi = 2 * pi;

  pladv (0);
  plvpor (0.0, 1.0, 0.0, 0.9);
  plwind (-1.0, 1.0, -0.9, 1.1);
  plcol0 (1);
  plw3d (1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, $alt[$k], $az[$k]);
  plbox3 (0.0, 0, 0.0, 0, 0.0, 0,
          "bnstu", "x axis", "bnstu", "y axis", "bcdmnstuv", "z axis");

  plcol0 (2);

#  x = r sin(phi) cos(theta)
#  y = r sin(phi) sin(theta)
#  z = r cos(phi)
#  r = 1 :=)

print $draw[0];

  for (my $i = 0; $i < 20; $i++ ) {
    my $theta = $two_pi * ($i + pdl [0, 0, 1, 1, 0]) / 20.;
    for (my $j = 0; $j < 20; $j++ ) {
      my $phi = pi * ($j + pdl [0, 1, 1, 0, 0]) / 20.1;
      my $x = sin ($phi) * cos ($theta);
      my $y = sin ($phi) * sin ($theta);
      my $z = cos ($phi);

      plpoly3 ($x, $y, $z, $draw->slice(",$k")->squeeze, 1);
    }
  }

  plcol0 (3);
  plmtex (1.0, 0.5, 0.5, "t", "unit radius sphere");

}

#--------------------------------------------------------------------------
# main

# Does a series of 3-d plots for a given data set, with different
# viewing options in each plot.
#--------------------------------------------------------------------------

use constant NPTS => 1000;

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

for (my $k = 0; $k < 4; $k++) {
  test_poly ($k);
}

# From the mind of a sick and twisted physicist...

my $i = sequence (NPTS);
my $z = -1. + 2. * $i / NPTS;

# Pick one ...

my $r = $z;

my $x = $r * cos (2 * pi * 6 * $i / NPTS);
my $y = $r * sin (2 * pi * 6 * $i / NPTS );

for ($k = 0; $k < 4; $k++) {
  pladv (0);
  plvpor (0.0, 1.0, 0.0, 0.9);
  plwind (-1.0, 1.0, -0.9, 1.1);
  plcol0 (1);
  plw3d (1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, $alt[$k], $az[$k]);
  plbox3 (0.0, 0, 0.0, 0, 0.0, 0,
	  "bnstu", "x axis", "bnstu", "y axis", "bcdmnstuv", "z axis");

  plcol0 (2);

  if ($opt[$k]) {
    plline3 ($x, $y, $z);
  } else {
    plpoin3 ($x, $y, $z, 1);
  }

  plcol0 (3);
  $title = sprintf ("#frPLplot Example 18 - Alt=%.0f, Az=%.0f",
                    $alt[$k], $az[$k]);
  plmtex (1.0, 0.5, 0.5, "t", title);
}

plend ();

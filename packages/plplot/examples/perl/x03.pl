#! /usr/bin/env perl
#
# Demo x03 for the PLplot PDL binding
#
# Polar plot demo
# (inspired from t/x03.t of module Graphics::PLplot, by Tim Jenness)
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

# SYNC: x03c.c 1.18

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

my $dtr = pi / 180.0;
my $x0 = cos ($dtr * sequence (361));
my $y0 = sin ($dtr * sequence (361));

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

# Set up viewport and window but do not draw box

plenv (-1.3, 1.3, -1.3, 1.3, 1, -2);

my ($x, $y);
for my $i (1 .. 10) {
  my $x = pdl (0.1 * $i * $x0);
  my $y = pdl (0.1 * $i * $y0);

  # Draw circles for polar grid

  plline ($x, $y);
}

plcol0 (2);
for my $i (0 .. 11) {
  my $theta = 30 * $i;
  my $dx = cos ( $dtr * $theta );
  my $dy = sin ( $dtr * $theta );

  # Draw radial spokes for polar grid

  pljoin (0, 0, $dx, $dy);

  # Write labels for angle
  # Slightly off zero to avoid floating point logic flips at 90 and 270 deg

  if ($dx >= -0.00001) {
    plptex ($dx, $dy, $dx, $dy, -0.15, int ($theta));
  } else {
    plptex ($dx, $dy, -$dx, -$dy, 1.15, int ($theta));
  }
}

# Draw the graph

my $r = sin ($dtr * 5 * sequence (361));
$x = $x0 * $r;
$y = $y0 * $r;

plcol0 (3);
plline ($x, $y);

plcol0 (4);
plmtex (2, 0.5, 0.5, "t", "#frPLplot Example 3 - r(#gh)=sin 5#gh");

# Close the plot at end

plend();

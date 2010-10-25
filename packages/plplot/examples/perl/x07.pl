#! /usr/bin/env perl
#
# Demo x07 for the PLplot PDL binding
#
# Font demo
#
# Copyright (C) 2004  Rafael Laboissiere
# (inspired from t/x07.t of module Graphics::PLplot, by Tim Jenness)
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

# SYNC: x07c.c 1.18

use PDL;
use PDL::Graphics::PLplot;

# Starting points for symbol lookup
my @base = (0, 200, 500, 600, 700, 800, 900, 2000, 2100, 2200, 2300,
            2400, 2500, 2600, 2700, 2800, 2900);

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

plfontld (1);

for my $base (@base) {

  pladv (0);

  # Set up viewport and window

  plcol0 (2);
  plvpor (0.15, 0.95, 0.1, 0.9);
  plwind (0, 1, 0, 1);

  # Draw the grid

  plbox (0.1, 0, 0.1, 0, "bcg", "bcg");

  # Write digits below the frame

  plcol0 (15);
  for my $i (0..9) {
    plmtex (1.5, (0.1 * $i + 0.05), 0.5, "b", $i);
  }

  my $k = 0;
 OUTER: for my $i (0..9) {

    # Write the digits to the left of the frame

    plmtex (1, (0.95 - 0.1 * $i), 1, "lv", ($base + 10 * $i));

    my $y = 0.95 - 0.1 * $i;
    for my $j (0..9) {

      my $x = 0.1 * $j + 0.05;

      plsym ($x, $y, ($k + $base));
      last OUTER if $k >= 127;
      $k++;
    }
  }

  plmtex (1.5, 0.5, 0.5, "t", "PLplot Example 7 - PLSYM symbols");

}

plend ();

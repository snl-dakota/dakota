#! /usr/bin/env perl
#
# Demo x06 for the PLplot PDL binding
#
# Font demo
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

# SYNC: x06c.c 1.16

use PDL;
use PDL::Graphics::PLplot;

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

pladv (0);

# Set up viewport and window

plcol0 (2);
plvpor (0.1, 1.0, 0.1, 0.9);
plwind (0.0, 1.0, 0.0, 1.3);

# Draw the grid using plbox

plbox (0.1, 0, 0.1, 0, "bcg", "bcg");

# Write the digits below the frame

plcol0 (15);

my ($i, $j, $k);

for ($i = 0; $i <= 9; $i++) {
  plmtex (1.5, (0.1 * $i + 0.05), 0.5, "b", $i);
}

$k = 0;

for ($i = 0; $i <= 12; $i++) {

  # Write the digits to the left of the frame

  plmtex (1.0, (1.0 - (2 * $i + 1) / 26.0), 1.0, "lv", 10 * $i);

  for ($j = 0; $j <= 9; $j++) {
    my $x = 0.1 * $j + 0.05;
    my $y = 1.25 - 0.1 * $i;

    # Display the symbols

    if ($k < 128) {
      plpoin ($x, $y, $k);
      $k++;
    }
  }
}

plmtex (1.5, 0.5, 0.5, "t", "PLplot Example 6 - plpoin symbols");

plend ();

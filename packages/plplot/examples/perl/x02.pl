#! /usr/bin/env perl
#
# Demo x02 for the PLplot PDL binding
#
# Multiple window and color map 0 demo
# (inspired from t/x02.t of module Graphics::PLplot, by Tim Jenness)
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

# SYNC: x02c.c 1.14

use PDL;
use PDL::Graphics::PLplot;

# Divide screen into 16 regions

plssub (4, 4);

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialise plplot

plinit ();

plschr (0.0, 3.5);
plfont (4);

for  (my $i = 0; $i <= 15 ; $i ++ ) {
  plcol0 ($i);
  my $text = $i;
  pladv (0);
  my $vmin = 0.1;
  my $vmax = 0.9;

  for (my $j = 0; $j <= 2; $j++ ) {
    plwid ($j + 1);
    plvpor ($vmin, $vmax, $vmin, $vmax);
    plwind (0.0, 1.0, 0.0, 1.0);
    plbox (0, 0, 0, 0, "bc", "bc");
    $vmin += 0.1;
    $vmax -= 0.1;
  }
  plwid (1);
  plptex (0.5, 0.5, 1.0, 0.0, 0.5, $text);
}

plend();

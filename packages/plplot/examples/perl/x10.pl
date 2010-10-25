#! /usr/bin/env perl
#
# Demo x10 for the PLplot PDL binding
#
# Window positioning demo.
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

# SYNC: x10c.c 1.13

use PDL;
use PDL::Graphics::PLplot;

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

pladv (0);
plvpor (0.0, 1.0, 0.0, 1.0);
plwind (0.0, 1.0, 0.0, 1.0);
plbox (0.0, 0, 0.0, 0, "bc", "bc");

plsvpa (50.0, 150.0, 50.0, 100.0);
plwind (0.0, 1.0, 0.0, 1.0);
plbox (0.0, 0, 0.0, 0, "bc", "bc");
plptex (0.5, 0.5, 1.0, 0.0, 0.5, "BOX at (50,150,50,100)");
plend ();

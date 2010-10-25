#! /usr/bin/env perl
#
# Demo x05 for the PLplot PDL binding
#
# Histogram demo
# (inspired from t/x05.t of module Graphics::PLplot, by Tim Jenness)
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

# SYNC: x05c.c 1.15

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

# Fill up data points

use constant NPTS => 2047;
my $delta = 2 * pi / NPTS;
my $data = sin (sequence (NPTS) * $delta);

plcol0 (1);
plhist ($data, -1.1, 1.1, 44, 0);
plcol0 (2);
pllab ("#frValue", "#frFrequency",
       "#frPLplot Example 5 - Probability function of Oscillator");

plend ();


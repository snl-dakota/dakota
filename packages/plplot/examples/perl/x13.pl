#! /usr/bin/env perl
#
# Demo x13 for the PLplot PDL binding
#
# Does a simple pie chart
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

# SYNC: x13c.c 1.15

# N.N. Although the visual results of this script is essentially the
# same as that produced by the equivalent C example x13c, the code
# below is quite different from that of x13c.c.  I use the vectorized
# operations of PDL extensively, such that most of the "for" loops
# disappeared. [R.L.]

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

my @text = (
  "Maurice",
  "Geoffrey",
  "Alan",
  "Rafael",
  "Vince"
);

my $per = pdl [10, 32, 12, 30, 16];

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

plenv (0, 10, 0, 10, 1, -2);
plcol0 (2);
# n.b. all theta quantities scaled by 2*pi/500 to be integers to avoid
# floating point logic problems.
my $theta0 = 0;
my $dthet = 1;
for (my $i = 0; $i <= 4; $i++) {
  # n.b. the theta quantities multiplied by 2*pi/500 afterward so
  # in fact per is interpreted as a percentage.
  my $theta1 = 5 * $per->index ($i);
  my $theta = $theta0 + sequence ($theta1->sclr + 1) * $dthet;
  my $xx = 5 + 3 * cos ((2 * pi / 500) * $theta);
  my $n = ($xx->dims)[0];
  my $x = zeroes ($n + 1);
  $x->slice("1:$n") .= $xx;
  $x->index (0) .= 5;
  my $yy = 5 + 3 * sin ((2 * pi / 500) * $theta);
  my $y = zeroes ($n + 1);
  $y->slice("1:$n") .= $yy;
  $y->index (0) .= 5;
  plcol0 ($i + 1);
  plpsty (($i + 3) % 8 + 1);
  plfill ($x, $y);
  plcol0 (1);
  plline ($x, $y);
  my $just = (2 * pi / 500) * ($theta0 + $theta1 / 2);
  my $dx = .25 * cos ($just);
  my $dy = .25 * sin ($just);
  $just = ((2 * $theta0 + $theta1) < 250
           || (2 * $theta0 + $theta1) > 750) ? 0 : 1;
  plptex (($xx->index ($n / 2) + $dx), ($yy->index ($n / 2) + $dy),
          1.0, 0.0, $just, $text[$i]);
  $theta0 += $theta1;
}
plfont (2);
plschr (0, 1.3);
plptex (5.0, 9.0, 1.0, 0.0, 0.5, "Percentage of Sales");

plend ();

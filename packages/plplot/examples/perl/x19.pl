#! /usr/bin/env perl
#
# Demo x19 for the PLplot PDL binding
#
# Illustrates backdrop plotting of world, US maps.
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

# SYNC: x19c.c 1.10

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

#--------------------------------------------------------------------------
# mapform19
#
# Defines specific coordinate transformation for example 19.
# Not to be confused with mapform in src/plmap.c.
# x[], y[] are the coordinates to be plotted.
#--------------------------------------------------------------------------

sub mapform19 {
  my ($x, $y) = @_;

  my $radius = 90.0 - $y;
  my $xp = $radius * cos ($x * pi / 180);
  my $yp = $radius * sin ($x * pi / 180);

  return ($xp, $yp);
}

#--------------------------------------------------------------------------
# main
#
# Shows two views of the world map.
#--------------------------------------------------------------------------

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Longitude (x) and latitude (y)

my $miny = -70;
my $maxy = 80;

plinit ();

# Cartesian plots
# Most of world

my $minx = 190;
my $maxx = 190 + 360;

plcol0 (1);
plenv ($minx, $maxx, $miny, $maxy, 1, -1);
plmap ($minx, $maxx, $miny, $maxy, 0, "usaglobe");

# The Americas

$minx = 190;
$maxx = 340;

plcol0 (1);
plenv ($minx, $maxx, $miny, $maxy, 1, -1);
plmap ($minx, $maxx, $miny, $maxy, 0, "usaglobe");

# Polar, Northern hemisphere

$minx = 0;
$maxx = 360;

plenv (-75., 75., -75., 75., 1, -1);
plmap ($minx, $maxx, $miny, $maxy, \&mapform19, "globe");

pllsty (2);
plmeridians (10.0, 10.0, 0.0, 360.0, -10.0, 80.0, \&mapform19);

plend ();

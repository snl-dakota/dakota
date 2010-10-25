#! /usr/bin/env perl
#
# Demo x11 for the PLplot PDL binding
#
# Mesh plot demo
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

# SYNC: x11c.c 1.24

use PDL;
use PDL::Graphics::PLplot;

use constant XPTS => 35;    # Data points in x
use constant YPTS => 46;    # Data points in y
use constant LEVELS => 10;

@opt = (DRAW_LINEXY, DRAW_LINEXY);

@alt = (33.0, 17.0);
@az = (24.0, 115.0);

@title = (
  "#frPLplot Example 11 - Alt=33, Az=24, Opt=3",
  "#frPLplot Example 11 - Alt=17, Az=115, Opt=3"
);

sub cmap1_init {

  my $i = pdl [0.0,    # left boundary
               1.0];   # right boundary

  my $h = pdl [240,    # blue -> green -> yellow ->
               0];     # -> red

  my $l = pdl [0.6, 0.6];

  my $s = pdl [0.8, 0.8];

  plscmap1n (256);
  plscmap1l(0, $i, $h, $l, $s, pdl ([]));
}

# main
#
# Does a series of mesh plots for a given data set, with different
# viewing options in each plot.

my $nlevel = LEVELS;

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

$z = zeroes (XPTS, YPTS);

my $x = 3 * (sequence (XPTS) - (XPTS / 2)) / (XPTS / 2);
my $y = 3 * (sequence (YPTS) - (YPTS / 2)) / (YPTS / 2);

# The code below may be vectorized to improve speed
for (my $i = 0; $i < XPTS; $i++) {
  my $xx = $x->index ($i);
  for (my $j = 0; $j < YPTS; $j++) {
    my $yy = $y->index ($j);
    $z->slice ("$i,$j") .=
      3. * (1.-$xx)*(1.-$xx) * exp(-($xx*$xx) - ($yy+1.)*($yy+1.)) -
      10. * ($xx/5. - pow($xx,3.) - pow($yy,5.)) * exp(-$xx*$xx-$yy*$yy) -
      1./3. * exp(-($xx+1)*($xx+1) - ($yy*$yy));
  }
}

$zmax = max ($z);
$zmin = min ($z);
$step = ($zmax - $zmin) / ($nlevel + 1);
$clevel = $zmin + $step + $step * sequence ($nlevel);

cmap1_init ();
for (my $k = 0; $k < 2; $k++) {
  for (my $i = 0; $i < 4; $i++) {
    pladv (0);
    plcol0 (1);
    plvpor (0.0, 1.0, 0.0, 0.9);
    plwind (-1.0, 1.0, -1.0, 1.5);
    plw3d (1.0, 1.0, 1.2, -3.0, 3.0, -3.0, 3.0, $zmin, $zmax,
           $alt[$k], $az[$k]);
    plbox3 (0.0, 0, 0.0, 0, 0.0, 4,
            "bnstu", "x axis", "bnstu", "y axis", "bcdmnstuv", "z axis");

    plcol0 (2);

    # wireframe plot
    if ($i == 0) {
      plmesh ($x, $y, $z, $opt[$k])

    # magnitude colored wireframe plot
    } elsif ($i == 1) {
      plmesh ($x, $y, $z,$opt[$k] | MAG_COLOR);

    # magnitude colored wireframe plot with sides
    } elsif ($i == 2) {
      plot3d ($x, $y, $z, $opt[$k] | MAG_COLOR, 1);

    # magnitude colored wireframe plot with base contour
    } elsif ($i == 3) {
      plmeshc ($x, $y, $z, $opt[$k] | MAG_COLOR | BASE_CONT, $clevel);
    }

    plcol0 (3);
    plmtex (1.0, 0.5, 0.5, "t", $title[$k]);
  }
}

plend();

#! /usr/bin/env perl
#
# Demo x08 for the PLplot PDL binding
#
# 3-d plot demo
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

# SYNC: x08c.c 1.45

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

use Getopt::Long;

use constant XPTS => 35;    # Data points in x
use constant YPTS => 46;    # Data points in y

use constant LEVELS => 10;

@alt = (60.0, 20.0);
@az = (30.0, 60.0);

@title = (
  "#frPLplot Example 8 - Alt=60, Az=30",
  "#frPLplot Example 8 - Alt=20, Az=60"
);

sub cmap1_init {
  my $gray = shift;

  my ($i, $h, $l, $s);

  $i = pdl [0.0,    # left boundary
            1.0];   # right boundary

  if ($gray) {
    $h = pdl [0.0,    # hue -- low: red (arbitrary if s=0)
              0.0];   # hue -- high: red (arbitrary if s=0)

    $l = pdl [0.5,    # lightness -- low: half-dark
              1.0];   # lightness -- high: light

    $s = pdl [0.0,    # minimum saturation
              0.0];   # minimum saturation
  } else {
    $h = pdl [240,    # blue -> green -> yellow -> */
              0];     # -> red

    $l = pdl [0.6, 0.6];
    $s = pdl [0.8, 0.8];
  }

  plscmap1n (256);
  plscmap1l (0, $i, $h, $l, $s, pdl []);
}

my $LEVELS = 10;

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);
GetOptions ("sombrero" => \$sombrero);

my $nlevel = LEVELS;
my $rosen = 1;

$rosen = 0
  if $sombrero;

# Initialize plplot

plinit ();

my $x = (sequence (XPTS) - (XPTS / 2)) / (XPTS / 2);
$x *=  1.5
  if $rosen;

my $y = (sequence (YPTS) - (YPTS / 2)) / (YPTS / 2);
$y += 0.5
  if $rosen;

my $z = zeroes (XPTS, YPTS);
my ($i, $j);
for ($i = 0; $i < XPTS; $i++) {
  my $xx = $x->index ($i);
  for ($j = 0; $j < YPTS; $j++) {
    my $yy = $y->index ($j);
    my $zz;
    if ($rosen) {
      $zz = (1 - $xx) ** 2 + 100 * ($yy - ($xx ** 2)) ** 2;
      # The log argument may be zero for just the right grid.
      if ($zz > 0.) {
        $zz = log ($zz);
      } else {
        $zz = -5.; # -MAXFLOAT would mess-up up the scale
      }
    }
    else {
      $r = sqrt ($xx * $xx + $yy * $yy);
      $zz = exp (-$r * $r) * cos (2.0 * pi * $r);
    }
    $z->index ($i)->index ($j) .= $zz;
  }
}

my $zmin = min ($z);
my $zmax = max ($z);
my $step = ($zmax - $zmin) / ($nlevel + 1);
my $clevel = $zmin + $step + $step * sequence ($nlevel);

pllightsource (1., 1., 1.);

for (my $k = 0; $k < 2; $k++) {
  for (my $ifshade = 0; $ifshade < 4; $ifshade++) {
    pladv (0);
    plvpor (0.0, 1.0, 0.0, 0.9);
    plwind (-1.0, 1.0, -0.9, 1.1);
    plcol0 (3);
    plmtex (1.0, 0.5, 0.5, "t", $title[$k]);
    plcol0(1);
    if ($rosen) {
      plw3d (1.0, 1.0, 1.0, -1.5, 1.5, -0.5, 1.5, $zmin, $zmax,
             $alt[$k], $az[$k]);
    } else {
      plw3d (1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, $zmin, $zmax,
             $alt[$k], $az[$k]);
    }

    plbox3 (0.0, 0, 0.0, 0, 0.0, 0,
            "bnstu", "x axis", "bnstu", "y axis", "bcdmnstuv", "z axis");
    plcol0 (2);

    if ($ifshade == 0) {        # diffuse light surface plot
      cmap1_init (1);
      plsurf3d ($x, $y, $z, 0, pdl []);
      } elsif ($ifshade == 1) { # magnitude colored plot
	cmap1_init (0);
	plsurf3d ($x, $y, $z, MAG_COLOR, pdl []);
      }
    elsif ($ifshade == 2) {     # magnitude colored plot with faceted squares
	cmap1_init (0);
	plsurf3d ($x, $y, $z, MAG_COLOR | FACETED, pdl []);
      } else {                  # magnitude colored plot with contours
	cmap1_init (0);
	plsurf3d ($x, $y, $z, MAG_COLOR | SURF_CONT | BASE_CONT, $clevel);
      }
  }
}


plend ();


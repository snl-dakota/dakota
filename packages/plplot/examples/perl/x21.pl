#! /usr/bin/env perl
#
# Demo x21 for the PLplot PDL binding
#
# Grid data demo
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

# SYNC: x21c.c 1.12

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];
use Getopt::Long qw [:config pass_through];
use POSIX qw [clock];
use PDL::GSL::RNG;

my $rng = PDL::GSL::RNG->new ('taus');

sub cmap1_init {
  my $i = pdl ([0.0,    # left boundary
                1.0]);  # right boundary

  my $h = pdl ([240,    # blue -> green -> yellow ->
                0]);    # -> red

  my $l = pdl ([0.6, 0.6]);

  my $s = pdl ([0.8, 0.8]);

  plscmap1n (256);
  plscmap1l (0, $i, $h, $l, $s, pdl ([]));
}

my ($xm, $ym, $xM, $yM);
my $randn = 0;
my $rosen = 0;

sub main {

  my $clev;

  # Options data structure definition. */

  my $pts = 500;
  my $xp = 25;
  my $yp = 20;
  my $nl = 15;
  my $knn_order = 20;
  my $threshold = 1.001;
  my $wmin = -1e3;

  GetOptions ("npts=i"      => \$pts,
              "randn"       => \$randn,
              "rosen"       => \$rosen,
              "nx=i"        => \$xp,
              "ny=i"        => \$yp,
              "nlevel=i"    => \$nl,
              "knn_order=i" => \$knn_order,
              "threshold=f" => \$threshold,
              "help"        => \$help);

  if ($help) {
    print (<<EOT);
$0 options:
    --npts points        Specify number of random points to generate [500]
    --randn              Normal instead of uniform sampling -- the effective
                         number of points will be smaller than the specified.
    --rosen              Generate points from the Rosenbrock function.
    --nx points          Specify grid x dimension [25]
    --ny points          Specify grid y dimension [20]
    --nlevel             Specify number of contour levels [15]
    --knn_order order    Specify the number of neighbors [20]
    --threshold float    Specify what a thin triangle is [1. < [1.001] < 2.]
EOT
    push (@ARGV, "-h");
  }

  unshift (@ARGV, $0);

  @title = ("Cubic Spline Approximation",
            "Delaunay Linear Interpolation",
            "Natural Neighbors Interpolation",
            "KNN Inv. Distance Weighted",
            "3NN Linear Interpolation",
            "4NN Around Inv. Dist. Weighted");

  @opt = (0., 0., 0., 0., 0., 0.);

  $xm = $ym = -0.2;
  $xM = $yM = 0.8;

  plParseOpts (\@ARGV, PL_PARSE_PARTIAL);

  $opt[2] = $wmin;
  $opt[3] = $knn_order;
  $opt[4] = $threshold;

  # Initialize plplot

  plinit ();

  my ($x, $y, $z) = create_data ($pts);    # the sampled data
  my $zmin = min ($z);
  my $zmax = max ($z);

  my ($xg, $yg) = create_grid ($xp, $yp);  # grid the data at
                                           # the output grided data

  my $xlab = "Npts=$pts gridx=$xp gridy=$yp";
  plcol0 (1);
  plenv ($xm, $xM, $ym, $yM, 2, 0);
  plcol0 (15);
  pllab ($xlab, "", "The original data");
  plcol0 (2);
  plpoin ($x, $y, 5);
  pladv (0);

  plssub (3, 2);

  for (my $k = 0; $k < 2; $k++) {
    pladv (0);
    for (my $alg = 1; $alg < 7; $alg++) {

      my $ct = clock ();
      $zg = plgriddata ($x, $y, $z, $xg, $yg, $alg, $opt [$alg - 1]);
      $xlab = sprintf ("time=\%d ms", (clock () - $ct) / 1000);
      $ylab = sprintf ("opt=\%.3f", $opt [$alg - 1]);

      # - CSA can generate NaNs (only interpolates?!).
      # - DTLI and NNI can generate NaNs for points outside the convex hull
      #     of the data points.
      # - NNLI can generate NaNs if a sufficiently thick triangle is not found
      #
      # PLplot should be NaN/Inf aware, but changing it now is quite a job...
      # so, instead of not plotting the NaN regions, a weighted average over
      # the neighbors is done.

      if ($alg == GRID_CSA || $alg == GRID_DTLI
          || $alg == GRID_NNLI || $alg == GRID_NNI) {

	for (my $i = 0; $i < $xp; $i++) {
	  for (my $j = 0; $j < $yp; $j++) {
	    if (not isfinite ($zg->slice ("$i,$j"))) {
              # average (IDW) over the 8 neighbors

	      $zg->slice ("$i,$j") .= 0;
              my $dist = 0;

	      for (my $ii = $i - 1; $ii <= $i + 1 && $ii < $xp; $ii++) {
		for (my $jj = $j - 1; $jj <= $j + 1 && $jj < $yp; $jj++) {
                  my $zgij = $zg->slice ("$ii,$jj");
		  if ($ii >= 0 && $jj >= 0 && isfinite ($zgij)) {
		    $d = (abs ($ii - $i) + abs ($jj - $j)) == 1 ? 1. : 1.4142;
		    $zg->slice ("$i,$j") .= $zg->slice ("$i,$j")
                      + $zgij / ($d * $d);
		    $dist += $d;
		  }
		}
	      }
	      if ($dist != 0.) {
		$zg->slice ("$i,$j") .= $zg->slice ("$i,$j") / $dist;
	      } else {
		$zg->slice ("$i,$j") .= $zmin;
              }
	    }
	  }
	}
      }

      my $lzM = max ($zg);
      my $lzm = min ($zg);

      plcol0 (1);
      pladv ($alg);

      if ($k == 0) {

        $lzm = min pdl ([$lzm, $zmin]);
        $lzM = max pdl ([$lzM, $zmax]);

        $clev = $lzm + ($lzM - $lzm) * sequence ($nl)/ ($nl - 1);

        plenv0 ($xm, $xM, $ym, $yM, 2, 0);
        plcol0 (15);
        pllab ($xlab, $ylab, $title [$alg - 1]);
        plshades ($zg, $xm, $xM, $ym, $yM,
                  $clev, 1, 0, 1, 1, 0, 0, 0);
        plcol0 (2);

      } else {

        $clev = $lzm + ($lzM - $lzm) * sequence ($nl)/ ($nl - 1);

	cmap1_init ();
	plvpor (0.0, 1.0, 0.0, 0.9);
	plwind (-1.0, 1.0, -1.0, 1.5);
	#
        # For the comparition to be fair, all plots should have the
        # same z values, but to get the max/min of the data generated
        # by all algorithms would imply two passes. Keep it simple.
        #
        # plw3d(1., 1., 1., xm, xM, ym, yM, zmin, zmax, 30, -60);
        #

	plw3d (1., 1., 1., $xm, $xM, $ym, $yM, $lzm, $lzM, 30, -60);
	plbox3 (0.0, 0, 0.0, 0, 0.0, 4,
                "bnstu", $ylab, "bnstu", $xlab, "bcdmnstuv", "");
	plcol0 (15);
	pllab ("", "", $title [$alg - 1]);
	plot3dc ($xg, $yg, $zg, DRAW_LINEXY | MAG_COLOR | BASE_CONT, $clev);
      }
    }
  }

  plend ();
}

sub create_grid {
  my ($px, $py) = @_;
  my ($x, $y);

  $x = $xm + ($xM - $xm) * sequence ($px) / ($px - 1);
  $y = $ym + ($yM - $ym) * sequence ($py) / ($py - 1);

  return ($x, $y);
}

sub create_data {
  my $pts = shift;
  my ($x, $y, $z);

  if (not $randn) {
    $x = $rng->get_uniform ($pts) + $xm;
    $y = $rng->get_uniform ($pts) + $ym;
  } else {    # std=1, meaning that many points are outside the plot range
    $x = $rng->ran_gaussian (1.0, $pts) + $xm;
    $y = $rng->ran_gaussian (1.0, $pts) + $ym;
  }

  if (not $rosen) {
    my $r = sqrt ($x ** 2 + $y ** 2);
    $z = exp (- $r ** 2) * cos (2 * pi * $r);
  } else {
    $z = log ((1 - $x) ** 2) + 100 * ($y - $x ** 2) ** 2;
  }

  return ($x, $y, $z);
}

main ();


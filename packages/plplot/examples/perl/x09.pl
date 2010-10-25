#! /usr/bin/env perl
#
# Demo x09 for the PLplot PDL binding
#
# Contour plot demo
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

# SYNC: x09c.c 1.27

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

use constant XPTS => 35;    # Data points in x
use constant YPTS => 46;    # Data points in y

use constant XSPA => 2 / (XPTS - 1);
use constant YSPA => 2 / (YPTS - 1);

# polar plot data
use constant PERIMETERPTS => 100;
use constant RPTS => 40;
use constant THETAPTS => 40;

# potential plot data
use constant PPERIMETERPTS => 100;
use constant PRPTS => 40;
use constant PTHETAPTS => 64;
use constant PNLEVEL => 20;

my $clevel = pdl [-1., -.8, -.6, -.4, -.2, 0, .2, .4, .6, .8, 1.];

# Transformation function

my @tr = (XSPA, 0.0, -1.0, 0.0, YSPA, -1.0);

sub mypltr {
  my ($x, $y) = @_;

  my $tx = $tr[0] * $x + $tr[1] * $y + $tr[2];
  my $ty = $tr[3] * $x + $tr[4] * $y + $tr[5];

  return ($tx, $ty);
}

# Polar contour plot example

sub polar {

  plenv (-1., 1., -1., 1., 0, -2);
  plcol0 (1);

  # Perimeter

  my $t = (2 * pi / (PERIMETERPTS - 1)) * sequence (PERIMETERPTS);
  my $px = cos ($t);
  my $py = sin ($t);

  plline ($px, $py);

  # Create data to be contoured

  my $r = ((sequence (RPTS)) / (RPTS - 1))->dummy (1, THETAPTS);
  my $theta = ((2 * pi / (THETAPTS - 1))
               * sequence (THETAPTS))->dummy (0, RPTS);

  my $cgrid2 = plAlloc2dGrid ($r * cos ($theta), $r * sin ($theta));
  my $z = $r;

  my $lev = 0.05 + 0.10 * sequence (10);

  plcol0 (2);
  plcont ($z, 1, RPTS, 1, THETAPTS, $lev, \&pltr2, $cgrid2);
  plcol0 (1);
  pllab ("", "", "Polar Contour Plot");

#  plFree2dGrid ($cgrid2);
}

# f2mnmx
#
# Returns min & max of input 2d array

sub f2mnmx {
  $f = shift;
  my $fmin = min ($f);
  my $fmax = max ($f);
  return ($fmin, $fmax)
}

# Shielded potential contour plot example

sub potential {

  # create data to be contoured

  my $r = (0.5 + sequence (PRPTS))->dummy (1, PTHETAPTS);
  my $theta = ((2 * pi / (PTHETAPTS - 1))
               * (0.5 + sequence (PTHETAPTS)))->dummy (0, PRPTS);

  my $xg = $r * cos ($theta);
  my $yg = $r * sin ($theta);

  my $cgrid2 = plAlloc2dGrid ($xg, $yg);

  my $rmax = 0.5 + (PRPTS - 1);

  my ($xmin, $xmax) = f2mnmx ($xg);
  my ($ymin, $ymax) = f2mnmx ($yg);

  my $x0 = ($xmin + $xmax) / 2;
  my $y0 = ($ymin + $ymax) / 2;

  # Expanded limits

  my $peps = 0.05;
  my $xpmin = $xmin - abs ($xmin) * $peps;
  my $xpmax = $xmax + abs ($xmax) * $peps;
  my $ypmin = $ymin - abs ($ymin) * $peps;
  my $ypmax = $ymax + abs ($ymax) * $peps;

  # Potential inside a conducting cylinder (or sphere) by method of images.
  # Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
  # Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
  # Also put in smoothing term at small distances.

  my $eps = 2;

  my $q1 = 1;
  my $d1 = $rmax / 4;

  my $q1i = - $q1 * $rmax / $d1;
  my $d1i = ($rmax ** 2) / $d1;

  my $q2 = -1;
  my $d2 = $rmax / 4;

  my $q2i = - $q2 * $rmax / $d2;
  my $d2i = ($rmax ** 2) / $d2;

  $div1 = sqrt (($xg - $d1) ** 2 + ($yg - $d1) ** 2 + $eps ** 2);
  $div1i = sqrt (($xg - $d1i) ** 2 + ($yg - $d1i) ** 2 + $eps ** 2);
  $div2 = sqrt (($xg - $d2) ** 2 + ($yg + $d2) ** 2 + $eps** 2);
  $div2i = sqrt (($xg - $d2i) ** 2 + ($yg + $d2i) ** 2 + $eps ** 2);
  my $z = $q1 / $div1 + $q1i / $div1i + $q2 / $div2 + $q2i / $div2i;

  my ($zmin, $zmax) = f2mnmx ($z);

  # Positive and negative contour levels

  my $dz = ($zmax - $zmin) / PNLEVEL;
  my $clevel = $zmin + (sequence (PNLEVEL) + 0.5) * $dz;;
  my $clevelneg = zeroes (PNLEVEL);
  my $clevelpos = zeroes (PNLEVEL);

  my $idx = which ($clevel <= 0);
  $clevelneg->index ($idx) .= $clevel->index ($idx);
  my $nlevelneg = $idx->dim (0);
  $idx = which ($clevel > 0);
  $clevelpos->index ($idx) .= $clevel->index ($idx);
  my $nlevelpos = $idx->dim (0);

  # Colours!

  my $ncollin = 11;
  my $ncolbox = 1;
  my $ncollab = 2;

  # Finally start plotting this page!

  pladv (0);
  plcol0 ($ncolbox);

  plvpas (0.1, 0.9, 0.1, 0.9, 1.0);
  plwind ($xpmin, $xpmax, $ypmin, $ypmax);
  plbox (0., 0, 0., 0, "", "");

  plcol0 ($ncollin);

  if ($nlevelneg > 0) {

    # Negative contours

    pllsty (2);
    plcont ($z, 1, PRPTS, 1, PTHETAPTS, $clevelneg, \&pltr2, $cgrid2);
  }

  if ($nlevelpos > 0) {

    # Positive contours

    pllsty (1);
    plcont ($z, 1, PRPTS, 1, PTHETAPTS, $clevelpos, \&pltr2, $cgrid2);
  }

  # Draw outer boundary

  my $t = (2 * pi / (PPERIMETERPTS - 1)) * sequence (PPERIMETERPTS);
  my $px = $x0 + $rmax * cos ($t);
  my $py = $y0 + $rmax * sin ($t);

  plcol0 ($ncolbox);
  plline ($px, $py);

  plcol0 ($ncollab);
  pllab ("", "", "Shielded potential of charges in a conducting sphere");

#  plFree2dGrid ($cgrid2);
}

# main
#
# Does several contour plots using different coordinate mappings

my $mark = 1500;
my $space = 1500;

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

# Set up function arrays

$xx = ((sequence (XPTS) - (XPTS / 2)) / (XPTS / 2))->dummy (1, YPTS);
$yy = ((sequence (YPTS) - (YPTS / 2)) / (YPTS / 2) - 1.0)->dummy (0, XPTS);
my $z = $xx * $xx - $yy * $yy;
my $w = 2 * $xx * $yy;

# Set up grids

$distort = 0.4;

my $xx = zeroes (XPTS);
my $yy = zeroes (YPTS);

for (my $i = 0; $i < XPTS; $i++) {
  for (my $j = 0; $j < YPTS; $j++) {
    my ($xij, $yij) = mypltr ($i, $j, 0);

    $xx->index ($i) .= $xij;
    $yy->index ($j) .= $yij;
  }
}

my $argx = $xx * pi / 2;
my $argy = $yy * pi / 2;

my $cgrid1 = plAllocGrid ($xx + $distort * cos ($argx),
                          $yy - $distort * cos ($argy));

$xx = $xx->dummy (1, YPTS);
$yy = $yy->dummy (0, XPTS);
$argx = $argx->dummy (1, YPTS);
$argy = $argy->dummy (0, XPTS);
my $cgrid2 = plAlloc2dGrid ($xx + $distort * cos ($argx) * cos ($argy),
                            $yy - $distort * cos ($argx) * cos ($argy));


# Plot using identity transform

pl_setcontlabelparam (0.006, 0.3, 0.1, 1);
plenv (-1.0, 1.0, -1.0, 1.0, 0, 0);
plcol0 (2);
plcont ($z, 1, XPTS, 1, YPTS, $clevel, \&mypltr, 0);
plstyl ($mark, $space);
plcol0 (3);
plcont ($w, 1, XPTS, 1, YPTS, $clevel, \&mypltr, 0);
plstyl (pdl([]), pdl([]));
plcol0 (1);
pllab ("X Coordinate", "Y Coordinate", "Streamlines of flow");
pl_setcontlabelparam (0.006, 0.3, 0.1, 0);

# Plot using 1d coordinate transform

plenv (-1.0, 1.0, -1.0, 1.0, 0, 0);
plcol0 (2);
plcont ($z, 1, XPTS, 1, YPTS, $clevel, \&pltr1, $cgrid1);
plstyl ($mark, $space);
plcol0 (3);
plcont ($w, 1, XPTS, 1, YPTS, $clevel, \&pltr1, $cgrid1);
plstyl (pdl([]), pdl([]));
plcol0 (1);
pllab ("X Coordinate", "Y Coordinate", "Streamlines of flow");

# Plot using 2d coordinate transform

plenv (-1.0, 1.0, -1.0, 1.0, 0, 0);
plcol0 (2);
plcont ($z, 1, XPTS, 1, YPTS, $clevel, \&pltr2, $cgrid2);

plstyl ($mark, $space);
plcol0 (3);
plcont ($w, 1, XPTS, 1, YPTS, $clevel, \&pltr2, $cgrid2);
plstyl (pdl([]), pdl([]));
plcol0 (1);
pllab ("X Coordinate", "Y Coordinate", "Streamlines of flow");

# pl_setcontlabelparam (0.006, 0.3, 0.1, 0);
polar ();

# pl_setcontlabelparam (0.006, 0.3, 0.1, 0);
potential ();

plend();

plFreeGrid ($cgrid1);
plFree2dGrid ($cgrid2);

#! /usr/bin/env perl
#
# Demo x22 for the PLplot PDL binding
#
# Simple vector plot example
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

# SYNC: x22c.c 1.8

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];

use constant nx => 20;
use constant ny => 20;
use constant nr => 20;
use constant ntheta => 20;
use constant nper => 100;
use constant nlevel => 10;

# Pairs of points making the line segments used to plot the user defined
# arrow
my $arrow_x = pdl [-0.5, 0.3, 0.3, 0.5, 0.3, 0.3];
my $arrow_y = pdl [0.0, 0.0,   0.2, 0.0, -0.2, 0.0];

sub f2mnmx {
  $f = shift;
  my $fmin = min ($f);
  my $fmax = max ($f);
  return ($fmin, $fmax)
}

# Vector plot of the gradient of a shielded potential (see example 9)

sub potential {
  # Potential inside a conducting cylinder (or sphere) by method of images.
  # Charge 1 is placed at (d1, d1), with image charge at (d2, d2).
  # Charge 2 is placed at (d1, -d1), with image charge at (d2, -d2).
  # Also put in smoothing term at small distances.

  my $rmax = nr;

  my $eps = 2;

  my $q1 = 1;
  my $d1 = $rmax / 4;

  my $q1i = - $q1 * $rmax / $d1;
  my $d1i = $rmax * $rmax / $d1;

  my $q2 = -1;
  my $d2 = $rmax / 4;

  my $q2i = - $q2 * $rmax / $d2;
  my $d2i = $rmax * $rmax / $d2;

  my $r = (0.5 + sequence (nr))->dummy (1, ntheta);
  my $theta = (2 * pi / (ntheta - 1) *
               (0.5 + sequence (ntheta)))->dummy (0, nr);
  my $x = $r * cos ($theta);
  my $y = $r * sin ($theta);
  my $cgrid2 = plAlloc2dGrid ($x, $y);
  my $div1 = sqrt (($x - $d1) ** 2 + ($y - $d1) ** 2 + $eps * $eps);
  my $div1i = sqrt (($x - $d1i) ** 2 + ($y - $d1i) ** 2 + $eps * $eps);
  my $div2 = sqrt (($x - $d2) ** 2 + ($y + $d2) ** 2 + $eps * $eps);
  my $div2i = sqrt (($x - $d2i) ** 2 + ($y + $d2i) ** 2 + $eps * $eps);
  my $z = $q1 / $div1 + $q1i / $div1i + $q2 / $div2 + $q2i / $div2i;
  my $u = -$q1 * ($x - $d1) / ($div1**3) - $q1i * ($x - $d1i) / ($div1i ** 3)
          -$q2 * ($x - $d2) / ($div2**3) - $q2i * ($x - $d2i) / ($div2i ** 3);
  my $v = -$q1 * ($y - $d1) / ($div1**3) - $q1i * ($y - $d1i) / ($div1i ** 3)
          -$q2 * ($y + $d2) / ($div2**3) - $q2i * ($y + $d2i) / ($div2i ** 3);

  my ($xmin, $xmax) = f2mnmx ($x);
  my ($ymin, $ymax) = f2mnmx ($y);
  my ($zmin, $zmax) = f2mnmx ($z);

  plenv ($xmin, $xmax, $ymin, $ymax, 0, 0);
  pllab ("(x)", "(y)",
         "#frPLplot Example 22 - potential gradient vector plot");

  # Plot contours of the potential
  my $dz = ($zmax - $zmin) / nlevel;
  my $clevel = $zmin + (sequence (nlevel) + 0.5) * $dz;

  plcol0 (3);
  pllsty (2);
  plcont ($z, 1, nr, 1, ntheta, $clevel, \&pltr2, $cgrid2);
  pllsty (1);
  plcol0 (1);

  # Plot the vectors of the gradient of the potential
  plcol0 (2);
  plvect ($u, $v, 25.0, \&pltr2, $cgrid2);
  plcol0 (1);

  # Plot the perimeter of the cylinder
  $theta = (2 * pi / (nper - 1)) * sequence (nper);
  my $px = $rmax * cos ($theta);
  my $py = $rmax * sin ($theta);
  plline ($px , $py);

  plFree2dGrid ($cgrid2);
}

#--------------------------------------------------------------------------
# main
#
# Generates several simple vector plots.
#--------------------------------------------------------------------------

# Parse and process command line arguments

plParseOpts (\@ARGV, PL_PARSE_SKIP | PL_PARSE_NOPROGRAM);

# Initialize plplot

plinit ();

my $npts = nx * ny;

my $xmin = -1.6;
my $xmax = 1.6;
my $ymin = -0.55;
my $ymax = 0.55;

# Create the data to plot
# - use the gradient of the rosen function in example 8

my $x = (1.5 * ((sequence (nx) - nx / 2) + 0.5)/(nx / 2))->dummy (1, ny);
my $y = (0.5 * ((sequence (ny) - ny / 2) + 0.5)/(ny / 2))->dummy (0, nx);
my $cgrid2 = plAlloc2dGrid ($x, $y);
my $u = -2 * (1 - $x) - 200 * ($y - $x ** 2);
my $v = 200 * ($y - $x * $x);

# Plot vectors using default arrow style
plenv ($xmin, $xmax, $ymin, $ymax, 0, 0);
pllab ("(x)", "(y)", "#frPLplot Example 22 - vector plot");
plcol0 (2);
plvect ($u, $v, -0.5, \&pltr2, $cgrid2);
plcol0 (1);

my $fill = 1;

# Create user defined arrow style and plot vectors using new style
plsvect ($arrow_x, $arrow_y, $fill);
plenv ($xmin, $xmax, $ymin, $ymax, 0, 0);
pllab ("(x)", "(y)", "#frPLplot Example 22 - filled arrow");
plcol0 (2);
plvect ($u, $v, 0.0005, \&pltr2, $cgrid2);
plcol0 (1);

# Example of polar plot

potential ();

plend ();

plFree2dGrid ($cgrid2);

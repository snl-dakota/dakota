#! /usr/bin/env perl
#
# Demo x16 for the PLplot PDL binding
#
# plshade demo, using color fill
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

# SYNC: x16c.c 1.26

use PDL;
use PDL::Graphics::PLplot;
use Math::Trig qw [pi];
use Getopt::Long qw(:config pass_through);
use Text::Wrap;

$Text::Wrap::columns = 72;

# Fundamental settings.  See @notes for more info.

use constant ns => 20;      # Default number of shade levels
use constant nx => 35;      # Default number of data points in x
use constant ny => 46;	    # Default number of data points in y
use constant exclude => 0;  # By default do not plot a page illustrating
			    # exclusion.  API is probably going to change
			    # anyway, and cannot be reproduced by any
			    # front end other than the C one.

# polar plot data
use constant PERIMETERPTS => 100;

# Transformation function

my @tr;

sub mypltr {
  my ($x, $y) = @_;

  my $tx = $tr[0] * $x + $tr[1] * $y + $tr[2];
  my $ty = $tr[3] * $x + $tr[4] * $y + $tr[5];

  return ($tx, $ty);
}

# --------------------------------------------------------------------------
#  f2mnmx
#
#  Returns min & max of input 2d array.
# --------------------------------------------------------------------------

sub f2mnmx {
  my $f = shift;

  my $fmin = min ($f);
  my $fmax = max ($f);

  return ($fmin, $fmax);
}

sub zdefined {
  my ($x, $y) = @_;

  my $z = sqrt ($x * $x + $y * $y);

  return ($z < 0.4 or $z > 0.6);
}

my @notes = (
  "To get smoother color variation, increase ns, nx, and ny.  To get faster",
  "response (especially on a serial link), decrease them.  A decent but quick",
  "test results from ns around 5 and nx, ny around 25."
);


# --------------------------------------------------------------------------
#  main
#
#  Does several shade plots using different coordinate mappings.
# --------------------------------------------------------------------------

my $fill_width = 2;
my $cont_color = 0;
my $cont_width = 0;

# Parse and process command line arguments

my ($exclude, $ns, $nx, $ny, $help);
$exclude = exclude;
$ns = ns;
$nx = nx;
$ny = ny;

GetOptions ("exclude" => \$exclude,
            "ns=i"    => \$ns,
            "nx=i"    => \$nx,
            "ny=i"    => \$ny,
            "help"    => \$help);

if ($help) {
  print (<<EOT);
$0 options:
    --exclude             Plot the "exclusion" page.
    --ns levels           Sets number of shade levels
    --nx xpts             Sets number of data points in x
    --ny ypts             Sets number of data points in y

EOT
  print (wrap ('', '', @notes), "\n");
  push (@ARGV, "-h");
}

unshift (@ARGV, $0);

plParseOpts (\@ARGV, PL_PARSE_PARTIAL);

# Reduce colors in cmap 0 so that cmap 1 is useful on a 16-color display

plscmap0n (3);

# Initialize plplot

plinit ();

# Set up transformation function

@tr = (2 / ($nx - 1), 0.0, -1.0, 0.0, 2 / ($ny - 1), -1.0);

my $x = ((sequence ($nx) - $nx / 2) / ($nx / 2))->dummy (1, $ny);
my $y = ((sequence ($ny) - $ny / 2) / ($ny / 2) - 1.0)->dummy (0, $nx);;

my $z = -sin (7 * $x) * cos (7 * $y) + $x ** 2 - $y ** 2;
my $w = -cos (7 * $x) * sin (7 * $y) + 2 * $x * $y;

my ($zmin, $zmax) =  f2mnmx ($z);

my $clevel = $zmin + ($zmax - $zmin) * (sequence ($ns) + 0.5) / $ns;
my $shedge = $zmin + ($zmax - $zmin) * sequence ($ns + 1) / $ns;

# Set up coordinate grids

my $distort = 0.4;

my $vx = sequence ($nx);
my $vy = sequence ($ny);
my ($x, $y) = mypltr ($vx->dummy (1, $ny), $vy->dummy (0, $nx));

my $xx = $x->slice (',0')->squeeze ();
my $yy = $y->slice ('0,')->squeeze ();
my $argx = $xx * pi / 2;
my $argy = $yy * pi / 2;

my $cgrid1 = plAllocGrid ($xx + $distort * cos ($argx),
                          $yy - $distort * cos ($argy));


my $argx = $x * pi / 2;
my $argy = $y * pi / 2;

my $cgrid2 = plAlloc2dGrid ($x + $distort * cos ($argx) * cos ($argy),
                            $y - $distort * cos ($argx) * cos ($argy));

# Plot using identity transform

pladv (0);
plvpor (0.1, 0.9, 0.1, 0.9);
plwind (-1.0, 1.0, -1.0, 1.0);

plpsty (0);

plshades ($z, -1., 1., -1., 1., $shedge, $fill_width,
          $cont_color, $cont_width, 1, 0, 0, 0);

plcol0 (1);
plbox (0.0, 0, 0.0, 0, "bcnst", "bcnstv");
plcol0 (2);
pllab ("distance", "altitude", "Bogon density");

# Plot using 1d coordinate transform

pladv (0);
plvpor (0.1, 0.9, 0.1, 0.9);
plwind (-1.0, 1.0, -1.0, 1.0);

plpsty (0);

plshades ($z, -1., 1., -1., 1., $shedge, $fill_width,
	  $cont_color, $cont_width, 1, 0, \&pltr1, $cgrid1);

plcol0 (1);
plbox (0.0, 0, 0.0, 0, "bcnst", "bcnstv");
plcol0 (2);
pllab ("distance", "altitude", "Bogon density");

# Plot using 2d coordinate transform

pladv (0);
plvpor (0.1, 0.9, 0.1, 0.9);
plwind (-1.0, 1.0, -1.0, 1.0);

plpsty (0);

plshades ($z, -1., 1., -1., 1., $shedge, $fill_width,
	  $cont_color, $cont_width, 1, 0, \&pltr2, $cgrid2);

plcol0 (1);
plbox (0.0, 0, 0.0, 0, "bcnst", "bcnstv");
plcol0 (2);
plcont ($w, 1, $nx, 1, $ny, $clevel, \&pltr2, $cgrid2);
pllab ("distance", "altitude", "Bogon density, with streamlines");

# Plot using 2d coordinate transform

pladv (0);
plvpor (0.1, 0.9, 0.1, 0.9);
plwind (-1.0, 1.0, -1.0, 1.0);

plpsty (0);

plshades ($z, -1., 1., -1., 1., $shedge, $fill_width,
          2, 3, 1, 0, \&pltr2, $cgrid2);

plcol0 (1);
plbox (0.0, 0, 0.0, 0, "bcnst", "bcnstv");
plcol0 (2);
pllab("distance", "altitude", "Bogon density");

# Plot using 2d coordinate transform and exclusion

if ($exclude) {
  pladv (0);
  plvpor (0.1, 0.9, 0.1, 0.9);
  plwind (-1.0, 1.0, -1.0, 1.0);

  plpsty (0);

  plshades ($z, -1., 1., -1., 1., $shedge, $fill_width,
	    $cont_color, $cont_width, 1,
            \&zdefined, \&pltr2, $cgrid2);

  plcol0 (1);
  plbox (0.0, 0, 0.0, 0, "bcnst", "bcnstv");

  pllab ("distance", "altitude", "Bogon density with exclusion");
}

plFreeGrid ($cgrid1);
plFree2dGrid ($cgrid2);

# Example with polar coordinates

pladv (0);
plvpor (.1, .9, .1, .9);
plwind (-1., 1., -1., 1.);

plpsty (0);

# Build new coordinate matrices

my $r = (sequence ($nx) / ($nx - 1))->dummy (1, $ny);
my $t = ((2 * pi * sequence ($ny) / ($ny - 1))->dummy (0, $nx));
$cgrid2 = plAlloc2dGrid ($r * cos ($t), $r * sin ($t));
$z = exp (- $r ** 2) * cos (5 * pi * $r) * cos (5 * $t);

# Need a new shedge to go along with the new data set

($zmin, $zmax) =  f2mnmx ($z);
$shedge = $zmin + ($zmax - $zmin) * sequence ($ns) / $ns;

# Now we can shade the interior region

plshades ($z, -1., 1., -1., 1., $shedge, $fill_width,
          $cont_color, $cont_width, 0, 0, \&pltr2, $cgrid2);

# Now we can draw the perimeter.  (If do before, shade stuff may overlap.)

$t = (2 * pi / (PERIMETERPTS - 1)) * sequence (PERIMETERPTS);
my $px = cos ($t);
my $py = sin ($t);

plcol0 (1);

plline ($px, $py);

# And label the plot

plcol0 (2);
pllab ("", "", "Tokamak Bogon Instability");

plend ();

plFree2dGrid ($cgrid2);

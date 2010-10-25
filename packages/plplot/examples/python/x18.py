#!/usr/bin/env python
#
#	x18c.c
#
#	3-d line and point plot demo.  Adapted from x08c.c.

import math
import pl
import sys
import os

module_dir = "@MODULE_DIR@"

if module_dir[0] == '@':
	module_dir = os.getcwd ()

sys.path.insert (0, module_dir)

opt = [1, 0, 1, 0]

alt = [20.0, 35.0, 50.0, 65.0]

az = [30.0, 40.0, 50.0, 60.0]

# main
#
# Does a series of 3-d plots for a given data set, with different
# viewing options in each plot.

NPTS = 1000

def main():

	# Parse and process command line arguments

	# pl.ParseOpts(sys.argv, pl.PARSE_FULL)

	# Initialize plplot

	pl.init()

	for k in range(4):
		test_poly(k)

	x = []
	y = []
	z = []

	# From the mind of a sick and twisted physicist...

	for i in range(NPTS):
		z.append(-1. + 2. * i / NPTS)

		# Pick one ...

##		r = 1. - ( (float) i / (float) NPTS )
		r = z[i]

		x.append(r * math.cos( 2. * math.pi * 6. * i / NPTS ))
		y.append(r * math.sin( 2. * math.pi * 6. * i / NPTS ))

	for k in range(4):
		pl.adv(0)
		pl.vpor(0.0, 1.0, 0.0, 0.9)
		pl.wind(-1.0, 1.0, -0.9, 1.1)
		pl.col(1)
		pl.w3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0,
		       alt[k], az[k])
		pl.box3("bnstu", "x axis", 0.0, 0,
			"bnstu", "y axis", 0.0, 0,
			"bcdmnstuv", "z axis", 0.0, 0)

		pl.col(2)

		if opt[k]:
			pl.line3(x, y, z)
		else:
			pl.poin3(x, y, z, 1)

		pl.col(3)
		title = "#frPLplot Example 18 - Alt=%.0f, Az=%.0f" % (alt[k],
								      az[k])
		pl.mtex("t", 1.0, 0.5, 0.5, title)

	pl.end()

def test_poly(k):

	draw = [ [ 1, 1, 1, 1 ],
		 [ 1, 0, 1, 0 ],
		 [ 0, 1, 0, 1 ],
		 [ 1, 1, 0, 0 ] ]

	pl.adv(0)
	pl.vpor(0.0, 1.0, 0.0, 0.9)
	pl.wind(-1.0, 1.0, -0.9, 1.1)
	pl.col(1)
	pl.w3d(1.0, 1.0, 1.0, -1.0, 1.0, -1.0, 1.0, -1.0, 1.0, alt[k], az[k])
	pl.box3("bnstu", "x axis", 0.0, 0,
		"bnstu", "y axis", 0.0, 0,
		"bcdmnstuv", "z axis", 0.0, 0)

	pl.col(2)

	def THETA(a):
		return 2. * math.pi * (a) / 20.
	def PHI(a):
		return math.pi * (a) / 20.1

##      x = r sin(phi) cos(theta)
##      y = r sin(phi) sin(theta)
##      z = r cos(phi)
##      r = 1 :=)

	for i in range(20):
		for j in range(20):
			x = []
			y = []
			z = []

			x.append(math.sin( PHI(j) ) * math.cos( THETA(i) ))
			y.append(math.sin( PHI(j) ) * math.sin( THETA(i) ))
			z.append(math.cos( PHI(j) ))

			x.append(math.sin( PHI(j) ) * math.cos( THETA(i+1) ))
			y.append(math.sin( PHI(j) ) * math.sin( THETA(i+1) ))
			z.append(math.cos( PHI(j) ))

			x.append(math.sin( PHI(j+1) ) * math.cos( THETA(i+1) ))
			y.append(math.sin( PHI(j+1) ) * math.sin( THETA(i+1) ))
			z.append(math.cos( PHI(j+1) ))

			x.append(math.sin( PHI(j+1) ) * math.cos( THETA(i) ))
			y.append(math.sin( PHI(j+1) ) * math.sin( THETA(i) ))
			z.append(math.cos( PHI(j+1) ))

			x.append(math.sin( PHI(j) ) * math.cos( THETA(i) ))
			y.append(math.sin( PHI(j) ) * math.sin( THETA(i) ))
			z.append(math.cos( PHI(j) ))

			# N.B.: The Python poly3 no longer takes a
			# (possibly negative) length argument, so if
			# you want to pass a counterclockwise polygon
			# you now have to reverse the list.  The code
			# above was rearranged to create a clockwise
			# polygon instead of a counterclockwise
			# polygon.

			pl.poly3(x, y, z, draw[k])

	pl.col(3)
	pl.mtex("t", 1.0, 0.5, 0.5, "unit radius sphere" )

main()

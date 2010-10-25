#!/usr/bin/env python
#
# tutor.c
# Tony Richardson
#
# This program is intended to be used as a template for creating simple
# two-dimensional plotting programs which use the PLplot plotting
# library.  The program was written with an emphasis on trying to clearly
# illustrate how to use the PLplot library functions.  
#
# This program reads data for M lines with N points each from an input
# data file and plots them on the same graph using different symbols.  It
# draws axes with labels and places a title at the top of the figure.  A
# legend is drawn to the right of the figure.  The input data file must
# have the following format:
#
#    x[1]     x[2]        x[3]        .     .     .     x[N]
#    y1[1]    y1[2]       y1[3]       .     .     .     y1[N]
#    y2[1]    y2[2]       y2[3]       .     .     .     y2[N]
#     .          .           .        .     .     .        .
#     .          .           .        .     .     .        .
#     .          .           .        .     .     .        .
#    yM[1]     yM[2]      yM[3]       .     .     .     yM[N]
#
# For example:
#
#	0	1	2	3	4
#	1.2	1.5	1.6	1.9	2.0
#	1.1	1.3	1.8	2.1	2.3
#	1.3	1.6	1.8	2.0	2.2
#
# (The first line contains the x-coordinates.  The succeeding M lines
# contain the y-coordinates of each of the M lines.)

import pl
import string
import sys

# You can select a different set of symbols to use when plotting the
# lines by changing the value of OFFSET. 

OFFSET = 2

def main():

	# Here are the character strings that appear in the plot legend.

	legend = ["Aardvarks", "Gnus", "Llamas"]

	# ==============  Read in data from input file. =============

	# Parse and process command line arguments

	pl.ParseOpts(sys.argv, pl.PARSE_FULL)

	# First prompt the user for the input data file name

	filename = raw_input("Enter input data file name.\n")

	# and open the file.

	try:
		datafile = open(filename, "r")
	except:
		error("Error opening input file.")

	# Read in all the data.

	try:
		lines = datafile.readlines()
		datafile.close()

		x = []
		data = string.split(lines[0])
		for num in data:
			x.append(string.atof(num))

		y = []
		del lines[0]
		for line in lines:
			yy = []
			data = string.split(line)
			for num in data:
				yy.append(string.atof(num))
			y.append(yy)
	except:
		error("Error while reading data file.")

	# ==============  Graph the data. =============

	# Set graph to portrait orientation. (Default is landscape.)
	# (Portrait is usually desired for inclusion in TeX documents.)

	pl.sori(-1)

	# Initialize plplot

	pl.init()

	# We must call pladv() to advance to the first (and only) subpage.
	# You might want to use plenv() instead of the pladv(), plvpor(),
	# plwind() sequence.
	
	pl.adv(0)

	# Set up the viewport.  This is the window into which the data is
	# plotted.  The size of the window can be set with a call to
	# plvpor(), which sets the size in terms of normalized subpage
	# coordinates.  I want to plot the lines on the upper half of the
	# page and I want to leave room to the right of the figure for
	# labelling the lines. We must also leave room for the title and
	# labels with plvpor().  Normally a call to plvsta() can be used
	# instead.

	pl.vpor(0.15, 0.70, 0.5, 0.9)

	# We now need to define the size of the window in user coordinates.
	# To do this, we first need to determine the range of the data
	# values.

	xmin, xmax = min(x), max(x)
	ymin = ymax = y[0][0]
	for yy in y:
		yymin, yymax = min(yy), max(yy)
		if yymin < ymin:
			ymin = yymin
		if yymax > ymax:
			ymax = yymax

	# Now set the size of the window. Leave a small border around the
	# data.

	xdiff = (xmax - xmin) / 20.
	ydiff = (ymax - ymin) / 20.
	pl.wind(xmin - xdiff, xmax + xdiff, ymin - ydiff, ymax + ydiff)

	# Call plbox() to draw the axes (see the PLPLOT manual for
	# information about the option strings.)

	pl.box("bcnst", 0.0, 0, "bcnstv", 0.0, 0)

	# Label the axes and title the graph.  The string "#gm" plots the
	# Greek letter mu, all the Greek letters are available, see the
	# PLplot manual.

	pl.lab("Time (weeks)", "Height (#gmparsecs)", "Specimen Growth Rate")

	# Plot the data.  plpoin() draws a symbol at each point.  plline()
	# connects all the points.

	i = 0
	for yy in y:
		pl.poin(x, yy, i + OFFSET)
		pl.line(x, yy)
		i = i + 1

	# Draw legend to the right of the chart.  Things get a little messy
	# here.  You may want to remove this section if you don't want a
	# legend drawn.  First find length of longest string.

	leglen = 0
	for leg in legend:
		j = len(leg)
		if j > leglen:
			leglen = j

	# Now build the string.  The string consists of an element from the
	# legend string array, padded with spaces, followed by one of the
	# symbols used in plpoin above.

	M = len(y)
	i = 0
	for leg in legend:
		if i >= M:
			break

		text = leg
		j = len(text)
		if j < leglen:		# pad string with spaces
			k = leglen - j
			text = text + ' ' * k

		# pad an extra space

		text = text + ' '

		# insert the ASCII value of the symbol plotted with plpoin()

		text = text + chr(i + OFFSET)

		# plot the string

		pl.mtex("rv", 1., 1. - float(i + 1) / (M + 1), 0., text)
		i = i + 1

	# Don't forget to call PLEND to finish off!

	pl.end()

def error(str):

	sys.stderr.write(str)
	sys.stderr.write('\n')
	sys.exit(1)

main()

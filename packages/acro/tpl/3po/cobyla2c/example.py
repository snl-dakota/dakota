#!/usr/bin/env python
# Python COBYLA example
# @(#) $Jeannot: example.py,v 1.2 2004/04/13 16:35:11 js Exp $

import cobyla

# A function to minimize
# Must return a tuple with the function value and the value of the constraints
# or None to abort the minimization
def function(x):
	f = x[0]**2+abs(x[1])**3
	# Two constraints to represent the equality constraint x**2+y**2 == 25
	con = [0]*2
	con[0] = x[0]**2 + x[1]**2 - 25 # x**2+y**2 >= 25
	con[1] = - con[0] # x**2+y**2 <= 25
	return f, con

# Optimizer call
rc, nf, x = cobyla.minimize(function, [-7, 3], low = [-10, 1], up = [3, 10])

print "After", nf, "function evaluations, COBYLA returned:", cobyla.RCSTRINGS[rc]
print "x =", x
print "f =", function(x)[0]
print "exact value = [-4.898979456, 1]"

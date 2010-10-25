# You may need to make minor adjustments to the assignment below.
# For example, you may want to use a different compiler, such as gcc.
# Some systems do not need -ldl.

# $S needs to point to a copy of the AMPL/solver interface library.
# The "S =" line assumes we are part of a DAKOTA source distribution.

S = ../../../packages/ampl
CFLAGS = -O
CC = cc

dakampl: dakampl.c
	$(CC) -o dakampl -I$S dakampl.c $S/amplsolver.a -lm -ldl

#!/usr/bin/env bash

# auxfiles option information:
#
# L   Ext  Description
# a  .adj  Constant in objective. This may have been eliminated by Presolve.
# c  .col  AMPL names of variables.
# f  .fix  AMPL variables fixed Presolve, including the value.
# r  .row  AMPL names of the constraints.
# s  .slc  AMPL names for eliminated constraints. These constraints can 
#          never be binding.
# u  .unv  AMPL variables not used in problem.

for x in *.mod; do
   base=`echo $x | sed 's/\.mod\$//'`;
   echo "model $base.mod; option auxfiles 'cr'; write g$base;" | ampl
done

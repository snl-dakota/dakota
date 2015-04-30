#!/bin/csh
echo "Creating MPS file."
../../src/example/knapMPS $1 > knapsack.mps
echo "Preparing to call CPLEX"
echo "read knapsack.mps" > cplex.commands
echo "set mip tolerances mipgap 1e-7" >> cplex.commands
echo "opt" >> cplex.commands
echo "quit" >> cplex.commands
echo "Calling CPLEX"
cplex < cplex.commands
echo "Cleaning up"
rm cplex.commands knapsack.mps cplex.log

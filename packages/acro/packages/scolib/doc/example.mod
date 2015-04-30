#
# An example AMPL model file.  After building coliny, this 
# can be executed by running
#
#   ampl example.mod
#
# in the coliny/src directory.
#
option solver coliny;
option coliny_options "solver=ps seed=100";
model ../etc/ampl/linear1.mod;
solve;

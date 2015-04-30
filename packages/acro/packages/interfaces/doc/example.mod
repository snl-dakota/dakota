#
# An example AMPL model file.  After building coliny, this 
# can be executed by running
#
#   ampl example.mod
#
# in the interfaces/src directory.
#
option solver acro;
option acro_options "solver=ps seed=100";
model ../../coliny/etc/ampl/linear1.mod;
solve;

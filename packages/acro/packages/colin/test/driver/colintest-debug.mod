model ../../etc/ampl/linear1.mod;
option solver colintest-debug;
option vg_options ("colintest");
option colintest_options (" solver=colin:test sufficient_objective_value=1.1 ");
solve;


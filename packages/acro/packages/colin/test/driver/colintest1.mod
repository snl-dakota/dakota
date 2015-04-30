model linear1.mod;
option solver "./colintest";
option colintest_options (" solver=colin:test sufficient_objective_value=1.1 ");
solve;

model linear2.mod;
option solver "./colintest";
option colintest_verbosity "verbose";
option colintest_options (" solver=colin:ls sufficient_objective_value=1.1 debug=9000 ");
solve;

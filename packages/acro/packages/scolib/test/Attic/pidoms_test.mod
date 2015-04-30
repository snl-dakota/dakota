model ../etc/ampl/linear3.mod;
option solver scolibtest;
option scolibtest_options "solver=scolib:pidoms";
solve;

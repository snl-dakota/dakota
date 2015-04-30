model ../../scolib/etc/ampl/text_book.mod;
option solver coliny;
option coliny_verbosity "verbose";
option coliny_options (" solver=optpp:cg max_iters=10 debug=1000 ");
solve;

var mass;
var a;
var v;
minimize force: mass*a;
minimize energy: 0.5 * mass * v^2;
option auxfiles rc;	# request stub.row and stub.col
write gfma;		# write stub.nl, stub.row, stub.col

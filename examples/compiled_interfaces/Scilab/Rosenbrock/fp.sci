//g = fp(x), gradient of Rosenbrock
function g = fp(x)
	g = zeros(2,1);
	f1 = 10*(x(2) - x(1)^2);
	g(1) = 2*x(1) - 2 - 40*f1*x(1);
	g(2) = 20*f1;
endfunction
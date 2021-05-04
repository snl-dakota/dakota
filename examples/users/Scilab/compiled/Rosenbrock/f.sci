// y = f(x) for Rosenbrock
function y = f(x)
	f1 = 10*(x(2) - x(1)^2);
	f2 = x(1) - 1;
	y = f1*f1 + f2*f2;
endfunction


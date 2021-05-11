%H = fpp(x), Hessian of Rosenbrock
function H = fpp(x)
H = zeros(2,2);
f1 = 10*(x(2) - x(1)^2);
H(1,1) = 2 - 40*(f1 - 20*x(1)^2);
H(1,2) = -400*x(1);
H(2,1) = H(1,2);
H(2,2) = 200;

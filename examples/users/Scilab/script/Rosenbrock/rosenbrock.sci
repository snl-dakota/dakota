// rosenbrock
//
// return function values for the rosenbrock problem
//		f( x1, x2 ) = alpha*( x2 - x1^2 )^2 + ( 1 - x1 )^2
//
// eventually: handle constraints, gradients, hessians
function f = rosenbrock(x,alpha)
  f = alpha*( x(2) - x(1)^2 )^2 + ( 1 - x(1) )^2;
endfunction

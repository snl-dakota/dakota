// bridge_optim_derivative.sci


function y = f(x)
	 [t,p,e,A,E,rho,F] = bridge_optim(x);
	 [U,P,R]= femtruss(bridge_optim, %F, x);

	 // The deck of the bridge is in IndexNodeVarInf
	 y = sqrt(sum(U(2*(IndexNodeVarInf-1)+2).^2));
endfunction

function dy = df(x)
	 dy = derivative(fobj_truss,x,order=Order)';
endfunction

//function [y, dy, ind] = optim_fobj_truss(x, ind)
//y  = fobj_truss(x);
//dy = dfobj_truss(x)';
//endfunction

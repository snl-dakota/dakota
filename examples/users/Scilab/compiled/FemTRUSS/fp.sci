function dy = df(x)
	 dy = derivative(fobj_truss,x,order=Order)';
endfunction

//function [y, dy, ind] = optim_fobj_truss(x, ind)
//y  = fobj_truss(x);
//dy = dfobj_truss(x)';
//endfunction

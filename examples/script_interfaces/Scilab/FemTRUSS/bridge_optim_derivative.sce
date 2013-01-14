lines(0);
warning_old = warning('query');

Order = 4; // 1 2 or 4 for derivative

femtruss_path = get_absolute_file_path('bridge_optim_derivative.sce');
exec(femtruss_path + 'build_long_bridge_2d.sce');

function y = fobj_truss(x)
[t,p,e,A,E,rho,F] = bridge_optim(x);
[U,P,R]= femtruss(bridge_optim, %F, x);

// The deck of the bridge is in IndexNodeVarInf
y = sqrt(sum(U(2*(IndexNodeVarInf-1)+2).^2));
endfunction

function dy = dfobj_truss(x)
dy = derivative(fobj_truss,x,order=Order)';
endfunction

function [y, dy, ind] = optim_fobj_truss(x, ind)
y  = fobj_truss(x);
dy = dfobj_truss(x)';
endfunction

////////////////////////////////////////////
// Parameters for the optimization method //
////////////////////////////////////////////

MaxEvalFunc = 400;
Algorithm   = 'gc'; // 'qn', 'gc', 'nd' -> Ne marche qu'avec 'qn' (quasi-newton). Pour les autres, on obtient rapidement une structure mal
                    // conditionnee
Log         = %F;
TOL         = 1.0e-6; // accuracy for convergence test (minimum)

//////////////////////////////
// Main part of the problem //
//////////////////////////////

printf('optimization starting, be patient ... \n\n');

[f_opt, x_opt] = optim(optim_fobj_truss, x0, algo=Algorithm,'ar',MaxEvalFunc,MaxEvalFunc,TOL,TOL);

printf('initial solution:'); disp(x0');
printf('initial objective function value = %f\n',fobj_truss(x0));

printf('Final solution:'); disp(x_opt');
printf('Final objective function value = %f\n',fobj_truss(x_opt));
  
scf();
plot_fobj_truss(x0);
xtitle('Before optimization','x','y');

scf();
plot_fobj_truss(x_opt);
xtitle('After optimization','x','y');

warning(warning_old);


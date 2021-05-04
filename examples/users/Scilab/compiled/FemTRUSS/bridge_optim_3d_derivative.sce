// Work done by Ms. Ibrahimcha, Nkamani, Hammad.

lines(0);
warning_old = warning('query');

warning('off');

Order = 4; // 1 2 or 4 for derivative
Debug = %T;
delta_h = 1e-6;

femtruss_path = get_absolute_file_path('bridge_optim_3d_derivative.sce');

//exec(femtruss_path + 'build_long_bridge_3d_bad.sce');
exec(femtruss_path + 'build_long_bridge_3d_good.sce');
  
function y = fobj_truss(x)
[t,p,e,A,E,rho,F] = bridge_optim_3d(x);
[U,P,R]= femtruss(bridge_optim_3d, %F, x);

// First objective: minimize the deformation at nodes of the deck with respect to y (coordinate 2)

// The deck of the bridge
Pos_deck  = localise3d(IndexNodeVarInf);

y = sqrt(sum(U(Pos_deck(2:3:$)).^2));
endfunction

function dy=dfobj_truss_diff(x)
dy = derivative(fobj_truss,x,delta_h,Order)';
endfunction

function [y, dy, ind] = optim_fobj_truss(x, ind)
y  = fobj_truss(x);
dy = dfobj_truss_diff(x);

if Debug then
  printf('y = %f\n', y);
  printf('norm(dy) = %f - abs(dy): max = %f min = %f\n', norm(dy), max(abs(dy)), min(abs(dy)));
end
endfunction

MaxEvalFunc = 40;
Algorithm   = 'gc'; // 'qn', 'gc', 'nd' -> Ne marche qu'avec 'qn' (quasi-newton). Pour les autres, on obtient rapidement une structure mal
                    // conditionnee
Tol = 1e-12;

////////////////////////////
// Start the Optimization //
////////////////////////////

printf('optimization starting, be patient ... \n\n');

tic();
//[f_opt, x_opt] = optim(optim_fobj_truss, 'b',lower, upper, x0, 'gc','ar',1000,1000,1e-6,1e-6);
//[f_opt, x_opt] = optim(optim_fobj_truss, 'b',lower, upper, x0, 'gc','ar',1000,1000,1e-12,1e-12);

// Be careful: the 'gc' algorithm doesn't seems to accept bounds constraint

[f_opt, x_opt] = optim(optim_fobj_truss, x0, Algorithm,'ar',MaxEvalFunc,MaxEvalFunc,Tol,Tol);

printf('initial solution:'); disp(x0');
printf('initial objective function value = %f\n',fobj_truss(x0));
printf('Final solution:'); disp(x_opt');
printf('Final objective function value = %f\n',fobj_truss(x_opt));
  
t_end=toc();
printf('computation time = %f\n' , t_end);

////////////////////////
// Plot the solutions //
////////////////////////

scf();
plot_fobj_truss(x0);
xtitle('Before optimization','x','y','z');

scf();
plot_fobj_truss(x_opt);
xtitle('After optimization','x','y','z');

warning(warning_old);


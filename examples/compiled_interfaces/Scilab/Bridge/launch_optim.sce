lines(0);

global Eval_Fobj;
Eval_Fobj = 0;
global Iteration;
Iteration = 0;

////////////////////
// Initialization //
////////////////////

bridge_path   = '/home/collette/Aster/Bridge/';
aster_path    = '/opt/stow/aster/';

as_run        = aster_path + 'ASTK/ASTK_SERV/bin/as_run';
gmsh          = aster_path + 'outils/gmsh';

tpl_file_name = 'bridge_geo_template.txt';
out_file_name = 'bridge.geo';

var_names     = ['x11','x12','y11','y12','z11','z12', ...
                 'x21','x22','y21','y22','z21','z22', ...
                 'x31','x32','y31','y32','z31','z32', ...
                 'x41','x42','y41','y42','z41','z42', ...
                 'x51','x52','y51','y52','z51','z52', ...
                 'x61','x62','y61','y62','z61','z62', ...
                 'x71','x72','y71','y72','z71','z72'];

// Initial structure
                 
x0 = [1.0, 1.0, 2.0, 2.0, 0.0, 1.0, ...
      2.0, 2.0, 2.0, 2.0, 0.0, 1.0, ...
      3.0, 3.0, 2.0, 2.0, 0.0, 1.0, ... 
      4.0, 4.0, 2.0, 2.0, 0.0, 1.0, ...
      5.0, 5.0, 2.0, 2.0, 0.0, 1.0, ... 
      6.0, 6.0, 2.0, 2.0, 0.0, 1.0, ...
      7.0, 7.0, 2.0, 2.0, 0.0, 1.0]'; 

// Boundaries for each coordinates

x_min = [0.0, 0.0, 1.1, 1.1, -1.0, -1.0, ...
         1.0, 1.0, 1.1, 1.1, -1.0, -1.0, ...
         2.0, 2.0, 1.1, 1.1, -1.0, -1.0, ... 
         3.0, 3.0, 1.1, 1.1, -1.0, -1.0, ...
         4.0, 4.0, 1.1, 1.1, -1.0, -1.0, ... 
         5.0, 5.0, 1.1, 1.1, -1.0, -1.0, ...
         6.0, 6.0, 1.1, 1.1, -1.0, -1.0]'; 

x_max = [2.0, 2.0, 10.0, 10.0, 2.0, 2.0, ...
         3.0, 3.0, 10.0, 10.0, 2.0, 2.0, ...
         4.0, 4.0, 10.0, 10.0, 2.0, 2.0, ... 
         5.0, 5.0, 10.0, 10.0, 2.0, 2.0, ...
         6.0, 6.0, 10.0, 10.0, 2.0, 2.0, ... 
         7.0, 7.0, 10.0, 10.0, 2.0, 2.0, ...
         8.0, 8.0, 10.0, 10.0, 2.0, 2.0]'; 

////////////////////////////
// The objective function //
////////////////////////////

// Main objective function

function [f] = fobj(x)
  var_values    = [x(1),  x(2),  x(3),  x(4),  x(5),  x(6), ...
                   x(7),  x(8),  x(9),  x(10), x(11), x(12), ...
                   x(13), x(14), x(15), x(16), x(17), x(18), ... 
                   x(19), x(20), x(21), x(22), x(23), x(24), ...
                   x(25), x(26), x(27), x(28), x(29), x(30), ... 
                   x(31), x(32), x(33), x(34), x(35), x(36), ...
                   x(37), x(38), x(39), x(40), x(41), x(42)]; 

  template_replace(tpl_file_name, var_names, var_values, out_file_name);
  [rep,stat,err]=unix_g(gmsh + ' -3 ' + bridge_path + out_file_name + ' -o ' + bridge_path + 'bridge.msh');
  if (stat~=0) then
    printf('rep = '); disp(rep);
    printf('err = '); disp(err);
  end

  timer(); // Timer initialization
  [rep,stat,err]=unix_g(as_run + ' ' + bridge_path + 'bridge.export');
  t2 = timer();

  if (stat~=0) then
    printf('rep = '); disp(rep);
    printf('err = '); disp(err);
  end

  fid   = mopen(bridge_path + 'Bridge_Result.dat','r');
  Lines = mgetl(fid);
  mclose(fid);
  Lines = evstr(Lines);
  f   = sum(Lines(:,2).^2) + sum(Lines(:,3).^2);

  global Eval_Fobj;
  Eval_Fobj = Eval_Fobj + 1;
  printf('Function evaluation %d: %f - %f sec\n', Eval_Fobj, f, t2);
endfunction

// A wrapper fobj for optim

function [f,g,ind] = fobj_optim(x,ind)
  global Iteration;
  Iteration = Iteration + 1;
  printf('Iteration %d starting\n', Iteration);
  f = fobj(x);
  g = derivative(fobj,x,h=1e-2);
  printf('Iteration %d finished\n', Iteration);
endfunction

///////////////
// Main part //
///////////////

t_start = getdate();
[f_opt, x_opt] = optim(fobj_optim,'b',x_min,x_max,x0,'gc','ar',length(x0)*100,100);
t_end = getdate();

unix('cp bridge.geo data/bridge_opt.geo');
unix('cp bridge.msh data/bridge_opt.msh');
unix('cp bridge_DEPL.pos data/bridge_DEPL.pos');
unix('cp bridge_SIGMA.pos data/bridge_SIGMA.pos');

printf('Time needed for optimization: %f sec\n',etime(t_end, t_start));

// Optimal solution found after 53 iterations (and 4505 function evaluations)

// x_opt = [0.0290807, 0.0290807, 1.1, 1.1, 0.0077895, 0.9922105, ...
//          2.2763263, 2.2763263, 2.042134, 2.042134, 0.1779691, ...
//     	    0.8220309, 3.5565186, 3.5565186, 2.5119591, 2.5119591, ...
//     	    0.4397569, 0.5602431, 4.0, 4.0, 2.601169, 2.601169, ...
//  	    0.5005940, 0.4994060, 4.4434814, 4.4434814, 2.5119591, ...
//     	    2.5119591, 0.4397569, 0.5602431, 5.7236737, 5.7236737, ...
//     	    2.042134, 2.042134, 0.1779691, 0.8220309, 7.9709193, ...
//     	    7.9709193, 1.1, 1.1, 0.0077895, 0.9922105]';

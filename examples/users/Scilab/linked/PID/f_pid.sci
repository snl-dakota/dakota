exec('init_test_automatic.sce');

// Definition of the objective function

function Dakota = f_pid(Dakota)

Dakota('failure') = 0;

if ( Dakota('numVars') ~= 2 | Dakota('numADIV') | Dakota('numADRV') ) then
  printf('Error: Bad number of variables in PID direct Scilab fn.\n');
  Dakota('failure') = 1;
elseif (Dakota('numFns') > 2) then
  // 1 fn -> opt, 2 fns -> least sq
  printf('Error: Bad number of functions in rosenbrock direct Scilab fn.\n');
  Dakota('failure') = 1;
else
  context.w0 = w0;
  context.m  = m;
  context.K0 = K0;
  context.P  = Dakota('xC')(1);
  context.I  = Dakota('xC')(2);

  Info = scicos_simulate(scs_m,list(),context,flag='nw');
  y_error = mean(abs((block_output('values')(:,1) -  block_output('values')(:,2))));
  y_diff  = mean(abs(diff(block_output('values')(:,2))));
  f0      = 0.5*y_error + 0.5*1*y_diff;

  // **** f:
  Dakota('fnVals') = f0;
  Dakota('fnLabels') = ['f1'];

  //pause(5); // for emulating a more expensive evaluation
end
endfunction

function Dakota = rosenbrock(Dakota, optarg1)

% optarg1 demonstrates passing a string from Dakota to Matlab
% disp(optarg1);

% possible convenience:
% assign all fields from Dakota structure to current (base) workspace
%field_names = fieldnames(Dakota); 
%num_fields = length(field_names);
%for i = 1:num_fields,
%  assignin('base', field_names{i}, getfield(Dakota,field_names{i}));
%end

Dakota.failure = 0;

if ( Dakota.numVars ~= 2 | Dakota.numADIV | Dakota.numADRV )
  sprintf('Error: Bad number of variables in rosenbrock direct Matlab fn.\n');
  Dakota.failure = 1;
elseif (Dakota.numFns > 2) 
  % 1 fn -> opt, 2 fns -> least sq
  sprintf('Error: Bad number of functions in rosenbrock direct Matlab fn.\n');
  Dakota.failure = 1;
else
 
  FUNCTION = 1;
  GRADIENT = 2;
  HESSIAN  = 4;

  if Dakota.numFns > 1 
    least_sq_flag = true;
  else
    least_sq_flag = false;
  end

  f0 = Dakota.xC(2)-Dakota.xC(1)*Dakota.xC(1);
  f1 = 1.-Dakota.xC(1);

  % **** f:
  if (least_sq_flag) 
    if bitand(Dakota.directFnASV(1), FUNCTION)
      Dakota.fnVals(1) = 10*f0;
    end
    if bitand(Dakota.directFnASV(2), FUNCTION)
      Dakota.fnVals(2) = f1;
    end
  else
    if bitand(Dakota.directFnASV(1), FUNCTION)
      Dakota.fnVals(1) = 100.*f0*f0+f1*f1;
    end
  end

  % **** df/dx:
  if (least_sq_flag)
    if bitand(Dakota.directFnASV(1), GRADIENT)
      Dakota.fnGrads(1,1) = -20.*Dakota.xC(1);
      Dakota.fnGrads(1,2) =  10.;
    end
    if bitand(Dakota.directFnASV(2), GRADIENT)
      Dakota.fnGrads(2,1) = -1.;
      Dakota.fnGrads(2,2) =  0.;
    end

  else 

    if bitand(Dakota.directFnASV(1), GRADIENT)
      Dakota.fnGrads(1,1) = -400.*f0*Dakota.xC(1) - 2.*f1;
      Dakota.fnGrads(1,2) =  200.*f0;
    end
    
  end

  % **** d^2f/dx^2:
  if (least_sq_flag)
    
    if bitand(Dakota.directFnASV(1), HESSIAN)
      Dakota.fnHessians(1,1,1) = -20.;
      Dakota.fnHessians(1,1,2) = 0.;
      Dakota.fnHessians(1,2,1) = 0.;
      Dakota.fnHessians(1,2,2) = 0.;
    end
    if bitand(Dakota.directFnASV(2), HESSIAN)
      Dakota.fnHessians(2,1:2,1:2) = 0.;
    end
    
  else
  
    if bitand(Dakota.directFnASV(1), HESSIAN)
      fx = Dakota.xC(2) - 3.*Dakota.xC(1)*Dakota.xC(1);
      Dakota.fnHessians(1,1,1) = -400.*fx + 2.0;
      Dakota.fnHessians(1,1,2) = -400.*Dakota.xC(1); 
      Dakota.fnHessians(1,2,1) = -400.*Dakota.xC(1);
      Dakota.fnHessians(1,2,2) =  200.;
    end
  
  end

  Dakota.fnLabels = {'f1'};

  %pause(5); // for emulating a more expensive evaluation

end

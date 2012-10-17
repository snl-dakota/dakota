function Dakota = rosenbrock(Dakota)

Dakota('failure') = 0;

if ( Dakota('numVars') ~= 2 | Dakota('numADIV') | Dakota('numADRV') ) then
  printf('Error: Bad number of variables in rosenbrock direct Scilab fn.\n');
  Dakota('failure') = 1;
elseif (Dakota('numFns') > 2) then
  // 1 fn -> opt, 2 fns -> least sq
  printf('Error: Bad number of functions in rosenbrock direct Scilab fn.\n');
  Dakota('failure') = 1;
else
  if Dakota('numFns') > 1 then
    least_sq_flag = %t;
  else
    least_sq_flag = %f;
  end

  f0 = Dakota('xC')(2)-Dakota('xC')(1)*Dakota('xC')(1);
  f1 = 1.-Dakota('xC')(1);

  // **** f:
  if (least_sq_flag) then
    if Dakota('directFnASM')(1,1) then
      Dakota('fnVals')(1) = 10*f0;
    end
    if Dakota('directFnASM')(1,2) then
      Dakota('fnVals')(2) = f1;
    end
  else
    if Dakota('directFnASM')(1,1) then
      Dakota('fnVals')(1) = 100.*f0*f0+f1*f1;
    end
  end

  // **** df/dx:
  if (least_sq_flag) then
    if Dakota('directFnASM')(2,1) then
      Dakota('fnGrads')(1,1) = -20.*Dakota('xC')(1);
      Dakota('fnGrads')(1,2) =  10.;
    end
    if Dakota('directFnASM')(2,2) then
      Dakota('fnGrads')(2,1) = -1.;
      Dakota('fnGrads')(2,2) =  0.;
    end

  else

    if Dakota.directFnASM(2,1) then
      Dakota('fnGrads')(1,1) = -400.*f0*Dakota('xC')(1) - 2.*f1;
      Dakota('fnGrads')(1,2) =  200.*f0;
    end
    
  end

    // **** d^2f/dx^2:
  if (least_sq_flag) then
    
    if Dakota('directFnASM')(3,1) then
      Dakota('fnHessians')(1)(1,1) = -20.;
      Dakota('fnHessians')(1)(1,2) = 0.;
      Dakota('fnHessians')(1)(2,1) = 0.;
      Dakota('fnHessians')(1)(2,2) = 0.;
    end
    if Dakota.directFnASM(3,2) then
      Dakota('fnHessians')(2)(1:2,1:2) = 0.;
    end
    
  else
  
    if Dakota('directFnASM')(3,1) then
      fx = Dakota('xC')(2) - 3.*Dakota('xC')(1)*Dakota('xC')(1);
      Dakota('fnHessians')(1)(1,1) = -400.*fx + 2.0;
      Dakota('fnHessians')(1)(1,2) = -400.*Dakota('xC')(1); 
      Dakota('fnHessians')(1)(2,1) = -400.*Dakota('xC')(1);
      Dakota('fnHessians')(1)(2,2) =  200.;
    end
  
  end

  Dakota('fnLabels') = ['f1'];

  //pause(5); // for emulating a more expensive evaluation
end
endfunction

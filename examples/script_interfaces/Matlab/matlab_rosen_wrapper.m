% matlab_rosen_wrapper.m
function matlab_rosen_wrapper(params,results)

%--------------------------------------------------------------
% Set any fixed/default values needed for your analysis .m code
%--------------------------------------------------------------

alpha = 100;


%------------------------------------------------------------------
% READ params.in (or params.in.num) from DAKOTA and set Matlab variables
%
% read params.in (no aprepro) -- just one param per line
% continuous design, then U.C. vars
% --> 1st cell of C has values, 2nd cell labels
% --> variables are in rows 2-->?
%------------------------------------------------------------------

fid = fopen(params,'r');
C = textscan(fid,'%n%s');
fclose(fid);

num_vars = C{1}(1);
	

% set design variables -- could use vector notation
% rosenbrock x1, x2

x(1) = C{1}(2);
x(2) = C{1}(3);


%------------------------------------------------------------------
% CALL your analysis code to get the function value
%------------------------------------------------------------------

[f] = rosenbrock(x,alpha);


%------------------------------------------------------------------
% WRITE results.out
%------------------------------------------------------------------
fid = fopen(results,'w');
fprintf(fid,'%20.10e     f\n', f);
%fprintf(fid,'%20.10e     params\n', C{1}(2:5));

fclose(fid);

% alternately
%save(results,'vContact','-ascii');

% matlab_beamGV_wrapper.m
function matlab_beamGV_wrapper(params,results)

%--------------------------------------------------------------
% Set any fixed/default values needed for your analysis .m code
%--------------------------------------------------------------

%------------------------------------------------------------------
% READ params.in (or params.in.num) from DAKOTA and set Matlab variables
%
% read params.in (no aprepro) -- just one param per line
% continuous design, then U.C. vars
% --> 1st cell of C has values, 2nd cell labels
% --> variables are in rows 2-->num_vars+1
%------------------------------------------------------------------

fid = fopen(params,'r');
C = textscan(fid,'%n%s');
fclose(fid);
	
% set design variables
num_vars = C{1}(1);
x = C{1}(2:num_vars+1);

%------------------------------------------------------------------
% CALL your analysis code to get the function value
%------------------------------------------------------------------

[f,g] = fbeamGV(x);
[gradf,gradg] = gbeamGV(x);
ncon=numel(g);

%------------------------------------------------------------------
% WRITE results.out
%------------------------------------------------------------------
fid = fopen(results,'w');
fprintf(fid,'%20.10e\n', f);
fprintf(fid,'%16.10e %16.10e %16.10e %16.10e %16.10e\n', g);
fprintf(fid,'\n');
%fprintf(fid,'%20.10e     params\n', C{1}(2:5));
fprintf(fid,'[ \n');
fprintf(fid,'%16.10e %16.10e %16.10e %16.10e %16.10e\n', gradf);
fprintf(fid,'] \n');
for j=1:ncon
	fprintf(fid,'[ \n');
	fprintf(fid,'%16.10e %16.10e %16.10e %16.10e %16.10e\n', gradg(:,j));
	fprintf(fid,'] \n');
end
fclose(fid);
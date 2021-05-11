// scilab_rosen_wrapper.sci
function scilab_rosen_wrapper(params,results)

exec('f.sci');
exec('fp.sci');
exec('build_long_bridge_2d.sce');
exec('bridge_optim_derivative.sce');

//--------------------------------------------------------------
// Set any fixed/default values needed for your analysis .sci code
//--------------------------------------------------------------

//alpha = 100;

//------------------------------------------------------------------
// READ params.in (or params.in.num) from DAKOTA and set Scilab variables
//
// read params.in (no aprepro) -- just one param per line
//------------------------------------------------------------------

fid = mopen(params,'r');
C = mgetl(fid,-1);
mclose(fid);

// set design variables -- could use vector notation

Line = tokens(C(2),' '); x(1) = eval(Line(1)); disp(x(1));
Line = tokens(C(3),' '); x(2) = eval(Line(1)); disp(x(2));

//------------------------------------------------------------------
// CALL your analysis code to get the function value
//------------------------------------------------------------------

f = f(x);
f2 = fp(x);

//------------------------------------------------------------------
// WRITE results.out
//------------------------------------------------------------------

fid = mopen(results,'w');
mfprintf(fid,'%20.10e     f\n', f);
mclose(fid);
endfunction

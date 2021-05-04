// Definition of the objective function

function y = f(x)
	context.w0 = w0;
	context.m  = m;
	context.K0 = K0;
	context.P  = x(1);
	context.I  = x(2);
	Info = scicos_simulate(scs_m,list(),context,flag='nw');
	y_error = mean(abs((block_output('values')(:,1) -  block_output('values')(:,2))));
	y_diff  = mean(abs(diff(block_output('values')(:,2))));
	y = 0.5*y_error + 0.5*1*y_diff; ...
endfunction

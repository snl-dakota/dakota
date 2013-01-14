loadXcosLibs();

importXcosDiagram('automatic_test.xcos');

// We find a good solution, but it doesn't take into account the stability of the system.
w0 = 2*%pi*100;
m  = 0.5;
K0 = 0.1;

context = [];
context.w0 = w0; ...
context.m  = m; ...
context.K0 = K0; ...
context.P = 233;
context.I = 0.59;

Info = scicos_simulate(scs_m,list(),context);

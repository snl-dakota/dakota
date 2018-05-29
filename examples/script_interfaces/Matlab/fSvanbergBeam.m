function [f,g] = fSvanbergBeam( x )
% Function evaluation of Svanberg's 5-segment beam
%
%--Input
%
% x....... Design variable vector = beam cros-sectional dimensions
%
%--Output
% f....... Objective function value f(x)=weight
% g....... Constraint function value g(x)=tip deflection constraint<=0

C1 = 0.0624;
C2 = 1;
f = C1*sum(x);
g = sum( [61 37 19 7 1]'./x(:).^3) - C2;

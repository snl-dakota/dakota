function [df,dg] = gSvanbergBeam( x )
% Gradient evaluation of Svanberg's 5-segment beam
%
%--Input
%
% x....... Design variable vector = beam cros-sectional dimensions
%
%--Output
% df...... Derivatives of objective function value f(x)=weight
% dg...... Derivatives of constraint function value g(x)=tip deflection

C1 = 0.0624;
df = repmat( C1, length(x), 1 );
dg = -3*[61 37 19 7 1]' ./ x(:).^4;

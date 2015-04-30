param n > 0 default 20;
set E := {(i,j) in V cross V : i < j};

set V := 1 .. n;
set Vp := 0 .. n-4;
set Vpp := 0 .. n-3;
set BigV := 0 .. n;
set BigVm := 0 .. n-1;

set Into{v in V} := {(i,j) in E : i = v or j = v};
set DeltSet := {(k,i,j) in Vp cross E : k < i};
set SmDeltSet := {(k,i,j) in DeltSet : k < i-1};
set BigDeltSet := {(k,i,j) in Vpp cross E : k < i};

var xs{(i,j) in E}; # := if (i,j) in HamE then 1 else 0;
var deltx{(k,i,j) in DeltSet};
var xx{(k,i,j) in BigDeltSet}; 
var x{E} >= 0;
var delty{(k,i,j) in BigVm cross E};
var yy{(k,i,j) in BigV cross E};
var y{E} >= 0;
var z;

maximize cut: z;

#very end isnt a triangle until enddegree put in!

subject to nonnegativity{(k,i,j) in SmDeltSet}: 
	deltx[k,i,j] >= 0;

subject to setdegree{(k,v) in Vp cross V: k >0 and k < v}:
	sum{(i,j) in Into[v] : (k-1,i,j) in DeltSet}deltx[k-1,i,j] = 0;

subject to initdegree{v in V}: 
	sum{(i,j) in Into[v]}xx[0,i,j] = z;

subject to enddegree{v in n-2 .. n}: 
	sum{(i,j) in Into[v]}xx[0,i,j] + 
	sum{(k,i,j) in DeltSet : (i,j) in Into[v]}
		deltx[k,i,j] = z;

subject to setdeltx{(k,i,j) in DeltSet diff SmDeltSet : k < n-3}: 
	deltx[k,i,j] = -sum{kbar in 0 .. k-1}deltx[kbar,i,j]-x[i,j];

subject to bounddeltx{(k,i,j) in DeltSet diff SmDeltSet : k < n-3}: 
	deltx[k,i,j] >= -z/2;

/*
subject to initxx{(i,j) in E}:
	xx[0,i,j] = x[i,j];

subject to setxx{(k,i,j) in BigV cross BigV cross BigV : 
	(k+1,i,j) in BigDeltSet and k < n-3}: 
	xx[k+1,i,j] = xx[k,i,j] + deltx[k,i,j];
*/

subject to nonnegativityy1{(k,i,j) in BigVm cross E : not((i,j) in 
			Into[k+1])}:
	delty[k,i,j] >= 0;
 
subject to nonnegativityy2{(k,i,j) in BigVm cross E : (i,j) in 
			Into[k+1]}:
	delty[k,i,j] <= 0;
 
subject to setdegreey{(k,v) in BigVm cross V : v != k+1}: 
	sum{(k,i,j) in BigVm cross E : (i,j) in Into[v]}
		delty[k,i,j] = 0;

subject to edgenonneg1{(i,j) in E}: 
	yy[0,i,j] + sum{ii in 0 .. i-1}delty[ii,i,j] >= 0;

subject to edgenonneg2{(i,j) in E}: 
	yy[0,i,j] + sum{jj in 0 .. j-1}delty[jj,i,j] >= 0;

subject to initx{(i,j) in E}: 
	xx[0,i,j] = yy[0,i,j] + sum{k in BigVm}delty[k,i,j];

subject to inity{(i,j) in E}: 
	y[i,j] >= yy[0,i,j];

subject to fixy{(i,j) in E}: 
	y[i,j] = xs[i,j];

#subject to setyy

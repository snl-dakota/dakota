##
## Peptide docking model
##

##
## PARAMETERS
##

#
# Number of sidechains
#
param n > 0 integer;

set SideChains := 1..n;

#
# Number of rotamers at each sidechain
#

param r {SideChains} >= 1 integer;
set Rotamers{i in SideChains} := 1..r[i];

set ValidIntraIndices within {i in SideChains, ri in Rotamers[i]};

set ValidInterIndices within {i in SideChains, ri in Rotamers[i], 
			  j in SideChains, rj in Rotamers[j] : 
			  i < j};
#
# Energy Values
#
param E_inter{ValidInterIndices};
param E_intra{ValidIntraIndices};

##
## VARIABLES
##
var delta {i in Sidechains, r in Rotamers[i]} binary;
var omega {e in ValidInteractions};


##
## OBJECTIVE
##
minimize energy:
	sum {i in SideChains, r in Rotamers[i]} E_inter[i,r] +
	sum {e in ValidInteractions} E_intra[e];

##
## CONSTRAINTS
##

SingleRotamer{i in SideChains}:
	sum {r in Rotamers[i]} delta[i,r] = 1;

BadInteraction{i in SideChains, ri in Rotamers[i], 
			  j in SideChains, rj in Rotamers[j] : 
			  (i,ri,j,rj) not in ValidInteractions}:
	delta[i,ri] + delta[j,rj] <= 1;

Omega1{(i,ri,j,rj) in ValidInteractions: E[i,ri,j,rj] < 0}:
	delta[i,ri] >= omega[i,ri,j,rj];

Omega2{(i,ri,j,rj) in ValidInteractions: E[i,ri,j,rj] < 0}:
	delta[j,rj] >= omega[i,ri,j,rj];

Omega3{(i,ri,j,rj) in ValidInteractions: E[i,ri,j,rj] > 0}:
	delta[i,ri] + delta[j,rj] - 1 <= omega[i,ri,j,rj];

Bound1{(i,ri,j,rj) in ValidInteractions}:
	omega[i,ri,j,rj] <= 1;


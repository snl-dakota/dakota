##
## Peptide docking model - extended to include AA arrays
##

##
## PARAMETERS
##

#
# Type of problem being solved
#  1 - minimize boltzman energies
#  2 - minimize rotamer energies
#
param ptype := 1;

#
# Number of sidechains
#
param n > 0 integer;
set SideChains := 1..n;
#
# Number of rotamers at each sidechain
#
param rCount {SideChains} >= 1 integer;
set Rotamers {i in SideChains} := 1..rCount[i];

param numAminoAcids := 20;
set aa := 1..20;

set ValidAminoAcids within {i in SideChains, a in aa};

##
## Setup for Boltzman Energy Values
##
set ValidInterIndicesAA := {i in SideChains, a in aa};
set ValidIntraIndicesAA := {i in SideChains, ai in aa,
  j in SideChains, aj in aa : i < j};
set InValidIntraIndicesAA := {i in SideChains, ai in aa,
  j in SideChains, aj in aa : i < j and (i,ai,j,aj) not in ValidIntraIndicesAA};
param B_inter{ValidInterIndicesAA};
param B_intra{ValidIntraIndicesAA};

##
## Setup for Rotamer Energy Values
##
set ValidInterIndicesRR within {i in SideChains, ri in Rotamers[i]};
set ValidIntraIndicesRR within {i in SideChains, ri in Rotamers[i], 
  j in SideChains, rj in Rotamers[j] :  i < j};
set InValidIntraIndicesRR := {i in SideChains, ri in Rotamers[i], 
  j in SideChains, rj in Rotamers[j] :  i < j and (i,ri,j,rj) not in ValidIntraIndicesRR};
param E_inter{ValidInterIndicesRR};
param RotamerLabel{ValidInterIndicesRR} within aa;
param E_intra{ValidIntraIndicesRR};


set ValidInterIndices := 
	if ptype == 1  then ValidInterIndicesAA
		       else {(i,ri) in ValidInterIndicesRR : (i,RotamerLabel[i,ri]) in ValidAminoAcids};
				
set ValidIntraIndices := 
	if ptype == 1  then ValidIntraIndicesAA
		       else {(i,ri,j,rj) in ValidIntraIndicesRR : (i,RotamerLabel[i,ri]) in ValidAminoAcids and (j,RotamerLabel[j,rj]) in ValidAminoAcids};
set InValidIntraIndices := 
	if ptype == 1  then InValidIntraIndicesAA
		       else {(i,ri,j,rj) in InValidIntraIndicesRR : (i,RotamerLabel[i,ri]) in ValidAminoAcids and (j,RotamerLabel[j,rj]) in ValidAminoAcids};

##
## VARIABLES
##
var delta {(i,ri) in ValidInterIndices} binary;
var omega {(i,ri,j,rj) in ValidIntraIndices} binary;
var IntraEnergy;
var InterEnergy;

##
## OBJECTIVE
##
minimize energy: InterEnergy + IntraEnergy;

##
## CONSTRAINTS
##
IntraC:
  IntraEnergy = if ptype == 1 then
		sum {(i,ri,j,rj) in ValidIntraIndices} omega[i,ri,j,rj] * B_intra[i,ri,j,rj]
		else
		sum {(i,ri,j,rj) in ValidIntraIndices} omega[i,ri,j,rj] * E_intra[i,ri,j,rj];

InterC:
  InterEnergy = if ptype == 1 then
		sum {(i,ri) in ValidInterIndices} delta[i,ri] * B_inter[i,ri]
		else
		sum {(i,ri) in ValidInterIndices} delta[i,ri] * E_inter[i,ri];

SingleRotamer{i in SideChains}:
  sum {(i,r) in ValidInterIndices} delta[i,r] = 1;

BadInteraction{(i,ri,j,rj) in InValidIntraIndices}:
  delta[i,ri] + delta[j,rj] <= 1;

SumConstraint {j in SideChains, (i,ri) in ValidInterIndices : i != j}:
  sum {(j,rj,i,ri) in ValidIntraIndices} omega[j,rj,i,ri] +
  sum {(i,ri,j,rj) in ValidIntraIndices} omega[i,ri,j,rj]  = delta[i,ri];

#Omega1{(i,ri,j,rj) in ValidIntraIndices: E_intra[i,ri,j,rj] < 0}:
  #delta[i,ri] >= omega[i,ri,j,rj];

#Omega2{(i,ri,j,rj) in ValidIntraIndices: E_intra[i,ri,j,rj] < 0}:
  #delta[j,rj] >= omega[i,ri,j,rj];

#Omega3{(i,ri,j,rj) in ValidIntraIndices: E_intra[i,ri,j,rj] > 0}:
  #delta[i,ri] + delta[j,rj] - 1 <= omega[i,ri,j,rj];

##
## Core peptide docking model
##

##
## Sets
##
set ValidSideChainIndices := {i in ValidSideChains, j in ValidSideChains : i<j};

set CompleteSideChainInteractions := {
	(i,ri) in ValidInterIndices, j in ValidSideChains 
	: ((i<j) and 
           (forall {rj in 1..NumRotamers[j]: (j,rj) in ValidInterIndices} 
		(i,ri,j,rj) in ValidIntraIndices))
	  or 
          ((i>j) and 
	   (forall {rj in 1..NumRotamers[j]: (j,rj) in ValidInterIndices} 
		(j,rj,i,ri) in ValidIntraIndices))};

set EnumeratedIndices;
let EnumeratedIndices := {-1};
set EnumeratedPoints dimen 3;
let EnumeratedPoints := {};

##
## VARIABLES
##
var delta {(i,ri) in ValidInterIndices} binary;
var omega {(i,ri,j,rj) in ValidIntraIndices} binary;
var omegaaux {(i,ri) in ValidInterIndices,  j in ValidSideChains 
		: (i,ri,j) not in CompleteSideChainInteractions and (i != j)};
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
  IntraEnergy = sum {(i,ri,j,rj) in ValidIntraIndices} 
			omega[i,ri,j,rj] * Energy_intra[i,ri,j,rj];

InterC:
  InterEnergy = sum {(i,ri) in ValidInterIndices} 
			delta[i,ri] * Energy_inter[i,ri];

SingleRotamer{i in ValidSideChains }:
  sum {(i,r) in ValidInterIndices} delta[i,r] = 1;

SumConstraint {j in ValidSideChains, (i,ri) in ValidInterIndices : i != j}:
  sum {(j,rj,i,ri) in ValidIntraIndices} omega[j,rj,i,ri] +
  sum {(i,ri,j,rj) in ValidIntraIndices} omega[i,ri,j,rj] + 
         (if ((i,ri,j) not in CompleteSideChainInteractions) 
	  then omegaaux[i,ri,j] ) 
  = delta[i,ri];

OmegaLower {(i,ri,j,rj) in ValidIntraIndices}:
  delta[i,ri] + delta[j,rj] -1 <= omega[i,ri,j,rj];

OmegaSum1 {(i,j) in ValidSideChainIndices}:
  sum {ri in 1..NumRotamers[i]: (i,ri,j) not in CompleteSideChainInteractions} 
	omegaaux[i,ri,j] <= 1;

OmegaSum2 {(i,j) in ValidSideChainIndices}:
  sum {rj in 1..NumRotamers[j]: (j,rj,i) not in CompleteSideChainInteractions} 
	omegaaux[j,rj,i] <= 1;

omegaauxLB {(i,ri) in ValidInterIndices,  j in ValidSideChains : 
		(i,ri,j) not in CompleteSideChainInteractions and (i != j)}:
  omegaaux[i,ri,j] >= 0;

enumerate {point in EnumeratedIndices : point >= 0}:
	sum {(point,i,r) in EnumeratedPoints} delta[i, r] 
						<= card(ValidSideChains)-1;

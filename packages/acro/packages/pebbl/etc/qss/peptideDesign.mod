##
## Peptide docking model for designing peptides
##

##
## PARAMETERS
##

##
## Core index sets for the formulation
##
set ValidInterIndices dimen 2;
set ValidIntraIndices within {(i,ai) in ValidInterIndices,
(j,aj) in ValidInterIndices : i < j};

param MaxNumRotamers := max {(i,ri) in ValidInterIndices} ri;
param MaxNumSideChains := max {(i,ri) in ValidInterIndices} i;
set ValidSideChains := {i in 1..MaxNumSideChains: exists {r in 1..MaxNumRotamers} (i,r) in ValidInterIndices};
param NumRotamers {i in ValidSideChains} := max {(i,ri) in ValidInterIndices} ri;


##
## Boltzman energy parameters
##
param Energy_inter{ValidInterIndices};
param Energy_intra{ValidIntraIndices};

##
## Include the constraint matrix
##
include peptideCore.mod;

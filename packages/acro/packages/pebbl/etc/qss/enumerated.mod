
model peptideDesign.mod;

include ampldata.mod;

option cplex_options 'integrality 1e-15 mipgap 1e-16';
solve;

#display delta;
#display omegaaux;

param k;
param flag;
let flag := 1;
param tmp_flag;
param thresh;
let k := 0;
# Enumerate 99 more times
repeat while (k < 99) and (flag == 1)
{
	for {(i,r) in ValidInterIndices}
	{
        if (delta[i,r] == 1) then
           let EnumeratedPoints := EnumeratedPoints union { (k,i,r) };
        }

        #display "**********************************************";
        printf "Solution %f",energy;
        let tmp_flag := 0;
        for {i in 1..MaxNumSideChains} {
	  if (i in ValidSideChains) then 
             for {(i,r) in ValidInterIndices} {
               if (delta[i,r] == 1) then {
	          printf " %d", r;
		  let tmp_flag := 1;
		  }
             }
          else printf " .";
	}
	if (tmp_flag == 0) then printf " Infeasible";
        printf "\n";
	let flag := tmp_flag;
 
        if (flag == 1) then {
        let EnumeratedIndices := EnumeratedIndices union { (k) };
	if (1 < 0) then {
        display k;
        display EnumeratedPoints;
        for {point in EnumeratedIndices : point >= 0} {
          display point, sum {(point,i,r) in EnumeratedPoints} delta[i, r];
	  }
	display card(ValidSideChains);
        display InterEnergy, IntraEnergy;
        display omega;
        display omegaaux;
        display CompleteSideChainInteractions;
        display ValidInterIndices;
        display ValidIntraIndices;
        for {(i,ri,j,rj) in ValidIntraIndices} {
          if (omega[i,ri,j,rj] == 1) then {display i,ri,j,rj, omega[i,ri,j,rj], floor(10000*Energy_intra[i,ri,j,rj]);}
        }
	}
        let thresh := energy + 10.0;
	option cplex_options ("integrality 1e-15 mipgap 1e-16 uppercutoff " & thresh);
        solve;
        let k := k + 1;
        }
}
        printf "Solution %f",energy;
        for {i in 1..MaxNumSideChains} {
	  if (i in ValidSideChains) then 
             for {(i,r) in ValidInterIndices} {
               if (delta[i,r] == 1) then {
	          printf " %d", r;
		  }
             }
          else printf " .";
	}
        printf "\n";


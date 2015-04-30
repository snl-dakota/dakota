option solver PICO;
model peptideDesign.mod;

include ampldata.mod;

solve;
#display delta;
#display omegaaux;

        printf "Solution %f",energy;
        for {i in 1..MaxNumSideChains} {
          if (i in ValidSideChains) then
             for {(i,r) in ValidInterIndices} {
               if (delta[i,r] == 1) then
                  printf " %d", r;
             }
          else printf " .";
        }
        printf "\n";

#option cplex_options 'integrality 1e-15 mipgap 1e-16';
#options PICO_options 'soplex=true clp=true';

#!/bin/csh

#set exp = `ls data/1abo*[0-9]`
set exp = `ls data/*boltz.out.single*[0-9]`
#set exp = `ls data/*out.test*[0-9]`
# data/*outall*[0-9]`

foreach i ($exp)
  echo "-------------------------------------------------------------"
  echo $i

  echo "A"
  (time serialBB --bound=0 $i) >&! ${i}.0.c
  echo "B"
  (time serialBB --bound=2 $i) >&! ${i}.2.c
  echo "A - enum"
  (time serialBB --bound=0 --EnumerationCount=1000 $i) >&! ${i}.0.c.enum
  echo "B - enum"
  (time serialBB --bound=2 --EnumerationCount=1000 $i) >&! ${i}.2.c.enum

  #echo "data $i ;" >! ampldata.mod
  #(time ampl run.mod) >&! ${i}.ip
  #(time ampl enumerated.mod) >&! ${i}.ip.enum

  ##
  ## Confirm that DEE did not change the final solution
  ##
  grep "  Solution:" ${i}.0.c | head -1 | awk '{for (i=4; i<=NF; i++) {printf "%s ", $i;} printf ":\n";}' >! ${i}.0.c.sol
  grep "  Solution:" ${i}.2.c | head -1 | awk '{for (i=4; i<=NF; i++) {printf "%s ", $i;} printf ":\n";}' >! ${i}.2.c.sol
  echo "DEE DIFF"
  diff ${i}.0.c.sol ${i}.2.c.sol
  ##
  ## Confirm that COMB and IP have the same final solution
  ##
  grep "  Solution:" ${i}.0.c | head -1 | awk '{for (i=4; i<=NF; i++) {printf "%s ", $i;} printf ":\n";}' >! ${i}.0.c.sol
  grep "Solution" ${i}.ip | grep -v "Infeasible" | awk '{for (i=3; i<=NF; i++) {printf "%s ", $i;} printf ":\n";}' >! ${i}.ip.sol
  echo "C/IP DIFF"
  diff ${i}.0.c.sol ${i}.ip.sol
  ##
  ## Confirm that DEE did not change the final solution for enumeration
  ##
  echo "# DEE ENUM DIFF"
  grep "  Solution:" ${i}.0.c.enum | awk '{for (i=4; i<=NF; i++) {printf "%s ", $i;} printf ":\n";}' >! ${i}.0.c.sol
  grep "  Solution:" ${i}.2.c.enum | grep -v "Infeasible" | awk '{for (i=4; i<=NF; i++) {printf "%s ", $i;} printf ":\n";}' >! ${i}.2.c.sol
  diff ${i}.0.c.sol ${i}.2.c.sol
  ##
  ## Confirm that COMB and IP have the same final solution
  ##
  grep "  Solution:" ${i}.0.c.enum | awk '{for (i=4; i<=NF; i++) {printf "%s ", $i;} printf ":\n";}' >! ${i}.0.c.sol
  grep "Solution" ${i}.ip.enum | grep -v "Infeasible" | awk '{for (i=3; i<=NF; i++) {printf "%s ", $i;} printf ":\n";}' >! ${i}.ip.sol
  echo "C/IP ENUM DIFF"
  echo "Not done"
  #diff ${i}.0.c.sol ${i}.ip.sol

  echo "#"
  echo "# Regular Stats"
  echo "#"
  @ nondeeval = `grep "Bounded" ${i}.0.c | awk '{print $2}'`
  @ deeval = `grep "Bounded" ${i}.2.c | awk '{print $2}'`
  @ tmp = $nondeeval - $deeval
  echo "Difference of " $tmp "subproblems (Comb0 - Comb2)"

  echo "# Comb time with DEE"
  set ctime = `grep "u " ${i}.0.c | awk '{print $1}' | sed 's/\(.*\)u/\1/'`
  echo "Comb0 Time: " $ctime
  set ctime2 = `grep "u " ${i}.2.c | awk '{print $1}' | sed 's/\(.*\)u/\1/'`
  echo "Comb2 Time: " $ctime2

  ##
  ## IP Time
  ##
  echo "# Difference of IP time with Comb0 time"
  set itime = `grep "u " ${i}.ip | awk '{print $1}' | sed 's/\(.*\)u/\1/'`
  echo "IP Time: " $itime
  set tmp = `echo "$ctime   $itime" | awk '{print $1 - $2;}'`
  echo "Difference: " $tmp

  ##
  ## Confirm that DEE changed the number of subproblems
  ##
  echo "#"
  echo "# Enumeration Stats"
  echo "#"
  @ nondeeval = `grep "Bounded" ${i}.0.c.enum | awk '{print $2}'`
  @ deeval = `grep "Bounded" ${i}.2.c.enum | awk '{print $2}'`
  @ tmp = $nondeeval - $deeval
  echo "Difference of " $tmp "subproblems (Comb0 - Comb2)"

  echo "# Comb time with DEE"
  set ctime = `grep "u " ${i}.0.c.enum | awk '{print $1}' | sed 's/\(.*\)u/\1/'`
  echo "Comb0 Time: " $ctime
  set ctime2 = `grep "u " ${i}.2.c.enum | awk '{print $1}' | sed 's/\(.*\)u/\1/'`
  echo "Comb2 Time: " $ctime2

  echo "# Difference of IP time with Comb0 time"
  set itime = `grep "u " ${i}.ip.enum | awk '{print $1}' | sed 's/\(.*\)u/\1/'`
  echo "IP Time: " $itime
  set tmp = `echo "$ctime   $itime" | awk '{print $1 - $2;}'`
  echo "Difference: " $tmp

  echo ""
end


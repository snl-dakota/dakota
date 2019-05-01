# Need: dprepro, python on path

# For debugging
#Set-PSDebug -Trace 1

# $1 and $2 are special variables in bash that contain the 1st and 2nd 
# command line arguments to the script, which are the names of the
# Dakota parameters and results files, respectively.

$params=$args[0]
$results=$args[1]

############################################################################### 
##
## Pre-processing Phase -- Generate/configure an input file for your simulation 
##  by substiting in parameter values from the Dakota paramters file.
##
###############################################################################

$pyprepro_py = (get-command "pyprepro").Path
python $pyprepro_py -I $params cantilever.template cantilever.i

############################################################################### 
##
## Execution Phase -- Run your simulation
##
###############################################################################


python ./cantilever cantilever.i > cantilever.log

############################################################################### 
##
## Post-processing Phase -- Extract (or calculate) quantities of interest
##  from your simulation's output and write them to a properly-formatted
##  Dakota results file.
##
###############################################################################

$mass         = (type cantilever.log | select -Last 15 | select -First 1).Trim().Split(" ")[0]
$stress       = (type cantilever.log | select -Last 11 | select -First 1).Trim().Split(" ")[0]
$displacement = (type cantilever.log | select -Last  7 | select -First 1).Trim().Split(" ")[0]

echo "$mass mass" | out-file -encoding ASCII $results
## echo "$stress stress" | out-file -encoding ASCII -Append $results
## echo "$displacement displacement" | out-file -encoding ASCII -Append $results

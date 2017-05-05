#!/bin/bash

###################################################################################################
# Exports variables in a Dakota parameters file as environment variables, making their values
# accessible in shell scripts without clunky string-based file parsing.
#
# The function export_dakota_params does all the work. It takes the name of a Dakota params file
# as an argument. After calling it, Dakota variables (and other non-variable items in the params
# file) are available as environment variables. The params file can be either default or aprepro
# formatted.
#
# * To provide some low-rent namespacing, environment variables are prefixed with DP_.
# * Bash variable names are restricted to contain alphanumeric characters and understcores, so 
#   characters such as colons (:) in expressions like "ASV:response_fn_1" are converted to 
#   underscores (_).
# * The double-quotes that string variables in aprepro formatted files are surrounded by are 
#   removed.
#
# EXAMPLE:
#  To print out the eval_id:
#
#    params=$1
#    source <this file>
#    export_dakota_params $params
#    echo ${DAK_eval_id}
#
###################################################################################################

export_dakota_params()
{
  # Expect params file as argument
  if [ ! $# -eq 1 ]; then
    echo "read_dakota_params: Expected (only) the name of a parameters file!"
    return 1
  fi
  local params=$1
  # params file must exist and be readable
  if [ ! -f $params ] || [ ! -r $params ] ; then
    echo "read_dakota_params: $params does not exist or is not readable"'!'
    return 1
  fi
  # Determine params file format, default or aprepro, by examining the first line
  # in the file. Set up a pair of functions that can extract the variable name ("key")
  # and value from a line. The functions are messy because string variable elements
  # can contain spaces and (in the aprepro case) are double-quoted.
  if head -1 $params | egrep -q '[0-9]+[[:space:]]+variables'; then
    _keyfunc() { echo $1 | awk '{print $NF}'; }
    _valfunc() { echo $1 | rev | cut -b $(expr length $2 + 1)- | rev; }
  elif head -1 $params | egrep -q 'DAKOTA_VARS     =[[:space:]]+[0-9]+'; then
    _keyfunc() { echo $1 | awk '{print $2}'; }
    _valfunc() { echo $1 | cut -b $(expr length $2 + 6)- | rev | cut -b 3- | rev | sed -e 's/"//g'; }
  else
    echo "read_dakota_params: $params is not a Dakota parameters file"'!'
    return 1
  fi
  # Export params file contents as bash variables. Variable names are DP_dakotaVarName.
  # (Colons are converted to underscores b/c of restrictions on bash/env vars)
  # An associative array would be a nicer sol'n, but arrays declared in functions
  # can't be made globally accessible until Bash 4.2, which is not yet commonly available.
  local key
  local value
  while read line; do
    key=$(_keyfunc "$line")
    value=$(_valfunc "$line" "$key")
    key="DAK_"$(echo $key | sed -r "s/[^[:alnum:]]/_/g")
    eval export $key=\"${value}\"
  done < $params
}


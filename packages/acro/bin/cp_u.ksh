#! /bin/ksh
#
# Copy files after first checking wether an update is necessary.
#

flags="-p"
flist=""
target=""
verbose=0

flag=1
while [ $# -gt 0 ] && [ $flag -eq 1 ];
do
  case "$1" in
    --v) verbose=1 ;;
    -*) flags="$flags $1"; shift; ;;
    *)  flag=0;;
  esac
done

nfiles=0
while [ $# -gt 1 ];
do
  flist="$flist $1";
  nfiles=$nfiles+1
  shift
done

target=$1

if [ "${flist}" = "" ]; then
   echo "ERROR: cp_u - missing target name"
   exit
fi

if [ ! -d ${target} ] && [ $nfiles -gt 1 ] ; then
   echo "ERROR: cp_u - target is a normal file, but more than one source file"
   echo "ERROR: cp_u -   has been specified."
   exit
fi

#echo "cp_u flags:  $flags"
#echo "cp_u flist:  $flist"
#echo "cp_u #flist: $nfiles"
#echo "cp_u target: $target"

if [ ! -d ${target} ] ; then
   if test $flist -nt $target -o ! -e $target ; then
      if [ $verbose -eq 1 ]; then
         echo "Copying $flist to $target"
      fi
      cp $flags $flist $target
   fi
else
   for file in ${flist}
   do
      fname=`basename ${file}`
      if test $file -nt $target/$fname -o ! -e $target/$fname ; then
         if [ $verbose -eq 1 ]; then
            echo "Copying $file to $target/$fname"
         fi
         cp $flags $file $target/$fname
      fi
   done
fi

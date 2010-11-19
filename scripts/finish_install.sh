#!/bin/sh

# A convenience script that can be run after configure/make/make
# install to finish placing DAKOTA files in the install directory
# specified by prefix

# To be run from build tree Dakota/ containing src, test, etc.

# uses variable:
UNAME=`uname`

if [ $# -lt 1 ]; then
  echo "usage: $0 prefix [distro_name]"
  exit
fi

prefix=$1
if [ $# -gt 1 ]; then
  distro_name=$2
else
  distro_name="ext_web"
fi

if [ ! -d $prefix ]; then
  echo "${0}: prefix directory does not exist; exiting"
  exit
fi
echo "${0}: using prefix: $prefix, and distro_name: $distro_name"

if [ ! -d $prefix/bin ]; then
  echo "${0}: making directory $prefix/bin"
  mkdir $prefix/bin
fi

# default linking binary (may be overridden below)
ln=/bin/ln
# default exeext
exeext=

# Platform-specific files
case $UNAME in

  *Linux*)
    # only do for nightly builds; not cluster installs
    HOST=`uname -n`
    if [ $HOST = "leoni.sandia.gov" ]; then
      echo "Copying DAKOTA's so dependencies to $prefix/bin"
      # for most deps
      deps3=`ldd ./src/dakota | awk '/=>/ {print $3}'` 
      # for ld-linux (probably no need to include)
      deps1=`ldd ./src/dakota | awk '! /=>/ {print $1}'` 
      for f in $deps3 $deps1; do
	if [ -f $f ]; then
          echo "Packaging: $f"
          cp -v $f $prefix/bin
	else
          echo "Could not find: $f"
	fi
      done
    fi
    # TODO: generate wrapper dakota.sh script
    ;;

  # TODO: Do not symlink binaries in Windows distributions
  # typical uname is "CYGWIN_NT-5.1"
  *CYGWIN*|*Cygwin*|*cygwin* )
    echo "Copying DAKOTA's CYGWIN DLL dependencies to $prefix/bin"
    for f in `cygcheck ./src/dakota.exe | grep cyg`; do
      echo $f
      cp `cygpath $f` $prefix/bin
    done
    # Do not perform links on Cygwin/MinGW as they break on Windows
    ln=/bin/true
    # override for windows
    exeext=".exe"
    ;;

  MINGW32_NT-5.1|*MINGW*|*mingw*)
    # Do not perform links on Cygwin/MinGW as they break on Windows
    ln=/bin/true
    # override for windows
    exeext=".exe"
    ;;

  # Include dependencies on Darwin (for now gcc RTL and openmpi)
  # May want to package all as we do for Linux?
  # typical uname is "Darwin"
  Darwin*|darwin* )
    echo "Copying DAKOTA's dylib dependencies to $prefix/bin"
    # prune first line beginning with the object being examined
    dakota_libs=`otool -L ./src/dakota | awk 'FNR > 1 {print $1}'` 
    # for now, this loop isn't needed; may be later
    all_libs="$dakota_libs"
    for lib1 in $dakota_libs; do
      secondary_libs=`otool -L $lib1 | awk 'FNR > 1 {print $1}'` 
      all_libs="$all_libs $secondary_libs" 
    done
    # don't include system libs
    dist_libs=`echo $all_libs | tr ' ' '\n' | sort -u | egrep -v "(^/System|^/usr/lib|^/usr/X11)"`
    for libf in $dist_libs; do
      if [ -f $libf ]; then
        echo "Packaging: $f"
        cp -v $libf $prefix/bin
      else
        echo "Could not find: $f"
      fi
    done
    cp $HOME/local/openmpi-1.3.3/LICENSE $prefix/bin
    ;;

esac

# eventually do the following in a loop over a read file?
# excerpts from extract_src.csh
echo "Copying additional DAKOTA files"

for f in INSTALL \
         LICENSE \
	 COPYRIGHT \
	 README \
         examples \
	 test; do
  cp -rf $f $prefix
done

cd $prefix
for d in examples test; do
  # TODO: this doesn't seem to work correctly on Solaris (sass9000), nor AIX
  #foreach f ( Makefile\* config\* \*.\[Cc\] \*.\[Ff\] \*.m4 \*.o )
  #  find $d -name "$f" -exec rm -f '{}' \;
  #end
  find $d -name 'Makefile*' -exec rm -f '{}' \;
  find $d -name 'config*' -exec rm -f '{}' \;
  find $d -name '*.[Cc]' -exec rm -f '{}' \;
  find $d -name '*.[Ff]' -exec rm -f '{}' \;
  find $d -name '*.m4' -exec rm -f '{}' \;
  find $d -name '*.o' -exec rm -f '{}' \;
done

# cleanup and repair soft links broken by moving src to bin during
# make install

cd $prefix/test
rm -rf .deps .libs
rm -f dakota$exeext dakota_library_mode$exeext \
    dakota_restart_util$exeext dakota_library_split$exeext
$ln -s ../bin/dakota$exeext dakota$exeext
$ln -s ../bin/dakota_restart_util$exeext dakota_restart_util$exeext
# dakota_library_mode only built in int_full
if [ -f ../bin/dakota_library_mode$exeext ]; then 
  $ln -s ../bin/dakota_library_mode$exeext dakota_library_mode$exeext
fi
# dakota_library_split only built in int_full
if [ -f ../bin/dakota_library_split$exeext ]; then 
  $ln -s ../bin/dakota_library_split$exeext dakota_library_split$exeext
fi

# only need to fix dakota and dakota_restart_util below, be cautious for now
cd $prefix/examples/tutorial
rm -rf .deps .libs
rm -f dakota$exeext rosenbrock$exeext text_book$exeext mogatest1$exeext
$ln -s ../../bin/dakota$exeext dakota$exeext
$ln -s ../../test/rosenbrock$exeext rosenbrock$exeext
$ln -s ../../test/text_book$exeext text_book$exeext
$ln -s ../../test/mogatest1$exeext mogatest1$exeext

cd $prefix/examples/methods
rm -rf .deps .libs
rm -f  dakota$exeext rosenbrock$exeext text_book$exeext cyl_head$exeext \
    container$exeext cantilever$exeext log_ratio$exeext mogatest2$exeext \
    mogatest3$exeext quasi_sine_fcn$exeext text_book_ouu$exeext
$ln -s ../../bin/dakota$exeext dakota$exeext
$ln -s ../../test/rosenbrock$exeext rosenbrock$exeext
$ln -s ../../test/text_book$exeext text_book$exeext
$ln -s ../../test/cyl_head$exeext cyl_head$exeext
$ln -s ../../test/container$exeext container$exeext
$ln -s ../../test/cantilever$exeext cantilever$exeext
$ln -s ../../test/log_ratio$exeext log_ratio$exeext
$ln -s ../../test/mogatest2$exeext mogatest2$exeext
$ln -s ../../test/mogatest3$exeext mogatest3$exeext
$ln -s ../../test/quasi_sine_fcn$exeext quasi_sine_fcn$exeext
$ln -s ../../test/text_book_ouu$exeext text_book_ouu$exeext

cd $prefix/examples/script_interfaces/generic
rm -rf .deps .libs
rm -f dakota$exeext rosenbrock$exeext 
$ln -s ../../../bin/dakota$exeext dakota$exeext
$ln -s ../../../test/rosenbrock$exeext rosenbrock$exeext

# Limit read access permissions for certain binaries and libraries
# when in int_supervised distribution mode
if [ "$distro_name" = "int_supervised" -o "$distro_name" = "int_full" ]; then
  echo "Setting permissions on supervised distribution"
  cd $prefix/bin
  chmod -f o-r dakota$exeext dakota_restart_util$exeext dakota_library_mode$exeext dakota_library_split$exeext
  cd $prefix/lib
  chmod -f o-r libdakota.a libdakota.la libdot.a libdot.la
fi

echo "${0}: done"

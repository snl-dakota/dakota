#
# CAUTION!
#
# This file should only be edited in the bootstrap/packages directory!
#

#
# TODO: Setup ACRO_BUILD_DATE macro (e.g. 20050221)
#


###############################################################
# boilerplate

AC_PREREQ(2.59)
AC_INIT([acro], [VOTD], [acro-help@software.sandia.gov])

rm -f packages/dummy/Makefile.in
if test -d packages ; then : ; else mkdir packages ; fi
if test -d packages/dummy ; then : ; else mkdir packages/dummy ; fi
touch packages/dummy/Makefile.in

###############################################################
# A file or directory that should be found here, so we know
# we are in the right place.

AC_CONFIG_SRCDIR([config/MakeMacros.mk])

###############################################################
# Location of our m4 macros.  These test for the features
# we are interested.  This doesn't actually work.  Documentation
# says it will in the future.  We need to define
# ACLOCAL_AMFLAGS in Makefile.am instead.  autoconf looks there
# for some reason.

AC_CONFIG_MACRO_DIR([config])
AC_CONFIG_AUX_DIR([config])

###############################################################
# Create this header file which will help configure our Makefiles
# and source files at build time.

AC_CONFIG_HEADERS([include/config.h])
AX_PREFIX_CONFIG_H([include/acro_config.h])
AH_BOTTOM(
#ifndef ACRO_HAVE_EXPLICIT
#ifndef explicit
#define explicit
#endif
#endif
#ifdef ACRO_USING_UTILIB
#include <utilib_config.h>
#endif
)




###############################################################
#
# Acro and cache files and MPI - issues
#
#    Acro has many options which are not compatible with cache
#    files.  These are options like --with-debugging which change
#    standard environment variables like CXXFLAGS.
#
#    Also, Acro has subpackages which should all be built with the
#    same compile environment.  In addition, Acro has subpackages
#    that must know whether or not they are being built with MPI.
#    The --enable-mpi option must appear on the configure command
#    line for these subpackages if MPI compilers are being used.
#    In addition, some subpackages are AC_CONFIG_SUBDIR'd and some
#    can not be, so we configure them ourselves from a Makefile.
#
#    These are the required behaviors:
#
#    If we are not using a cache file (use_ac_cv_env=no):
#
#      Run through all our build configure options, and assign
#      values to the standard build environment variables.
#
#      Export the standard build environment variables so that
#      AC_CONFIG_SUBDIR'd packages can find them.
#
#      Write the standard build environment variables to a Makefile
#      variable (ENV_ARGS) so we can include them when we configure 
#      subpackages from a Makefile (mtl, coin-cbc and trilinos)
#
#    If we are getting our build environment from a cache file
#    (use_ac_cv_env=yes and ac_cv_env_empty=no):
#
#      Skip all of our configure options which alter the standard
#      build environment variables.
#
#      If we detect that MPI is being used (either --enable-mpi
#      was specified, or we had previously saved mpi command line
#      arguments in a cache file) then set the HAVE_MPI macro and 
#      the BUILD_MPI Makefile variable.
#
#      Add --enable-mpi to the configure command line of subpackages
#      that we configure that need it.  (trilinos only at present)
#
#      Pass along the --cache-file= or -C configure command line
#      option to subpackages that we configure.  (mtl, coin-cbc and trilinos)
#
#      Add --enable-mpi to our configure command line if it is not
#      there (so AC_CONFIG_SUBDIR'd packages that need it will know 
#      they are using MPI) (for appspack and utilib)
#
#    If a cache file is specified, but doesn't exist yet
#    (use_ac_cv_env=yes and ac_cv_env_empty=yes):
#
#      Do everything we do if no cache file is specified.
#
#      Pass along the --cache-file= or -C configure command line
#      option to subpackages that we configure.  (mtl, coin-cbc and trilinos)
#     
#      Set the standard cache file variables (ac_cv_env*) to the 
#      build environment variables, and set special MPI variables
#      (acro_cv_with_mpi_*) in the cache file.
#
#  Subpackages that we configure: mtl, coin-cbc and trilinos
#  Subpackages that require --enable-mpi in order to know that
#  they are using MPI: utilib, appspack and trilinos
#

# Are we using a cache file, and if so does the cache file already
# exist, or is it empty and we are initializing it?

use_ac_cv_env=no
ac_cv_env_empty=yes
CACHE_FILE_ARG=""

if test "$cache_file" != "/dev/null" ; then
  use_ac_cv_env=yes
  CACHE_FILE_ARG="--cache-file=$PWD/$cache_file"
  if test X$ac_cv_env_CCASFLAGS_set = Xset ; then
    ac_cv_env_empty=no
  fi
fi

#For mtl, coin-cbc and trilinos
AC_SUBST([CACHE_FILE_ARG])

if test "$ac_cv_env_empty" = "no" ; then
  AC_MSG_NOTICE([=====================================================================])
  AC_MSG_NOTICE([Compile environment obtained from standard GNU environment variables.])
  AC_MSG_NOTICE([Acro's special options for the compile environment])
  AC_MSG_NOTICE([(for example "--with-debugging") will be ignored.])
  AC_MSG_NOTICE([=====================================================================])
fi

###############################################################
# Setup platform information and initialize automake

AC_CANONICAL_BUILD
AC_CANONICAL_HOST
AC_CANONICAL_TARGET
AM_INIT_AUTOMAKE([foreign -Wall -Werror filename-length-max=256 tar-ustar])
#AM_INIT_AUTOMAKE([1.9 foreign -Wall -Werror tar-ustar])


###############################################################
# Platform ID

ACRO_PLATFORM


###############################################################
# Configure command line options
# Default compilation flags: -O2 plus maybe some arch/compiler
# special flags

ACRO_BOOST_DEPENDENCE([available])

# --with-compiler=mingw
ACRO_COMPILER_OPTION

if test $ac_cv_env_empty = yes ; then
  # Various special options to override compiler flags.
  # Only legal if we're not using cache file settings.
  ACRO_COMPILER_OPTIONS
fi

# Control whether UTILIB DEBUGPR output is used
ACRO_DEBUGPR_OPTION
# Control whether UTILIB CommonIO mapping is used
ACRO_COMMONIO_OPTION
# Control whether UTILIB memory debugging utilities are used
ACRO_MEMDEBUG_OPTION
# Control whether UTILIB checksum is used
ACRO_CHECKSUM_OPTION
# Control whether executables are built
ACRO_EXECUTABLES_OPTION
# Control whether test codes are built
ACRO_TEST_OPTION
# Control whether code validation is used
ACRO_VALIDATING_OPTION

###############################################################

# User can set --enable-mpi.  It will be set to "yes" automatically
# if they specify any of the --with-mpi-* options.
#
# Also, DAKOTA sets --enable-mpi when it builds Acro with a cache
# file, so we know whether or not we are building with MPI
#
AC_ARG_ENABLE(mpi,
AS_HELP_STRING([--enable-mpi],[Enable MPI support]),
[ENABLE_MPI=$enableval],
[ENABLE_MPI=no]
)

if test $ac_cv_env_empty = yes ; then
  # Various options to setup the use of MPI.
  # Only legal if we're not using cache file settings.
  # Creates cache variables for --with-mpi-* options that will go
  # to cache file if a cache file is created.
  ACRO_MPI_OPTIONS
fi 

if test "X$ENABLE_MPI" = "Xyes" ; then
  using_mpi=yes
elif test "X$acro_cv_with_mpi_set" = "Xyes" ; then
  using_mpi=yes
else
  using_mpi=no
fi

#For utilib and appspack (actually utilib is smart enough to look
# in the cache file for acro_cv_with_mpi_* variables).
if test "$using_mpi" = "yes" && test "X$ENABLE_MPI" != "Xyes" ; then
  new_configure_args="$ac_configure_args --enable-mpi"
  ac_configure_args="$new_configure_args"
fi

#
# Hard-coding UTILIB configuration
#
pwd=`pwd`
if test -d tpl/cxxtest; then
   new_configure_args="$ac_configure_args --with-cxxtest=$pwd/tpl/cxxtest"
   ac_configure_args="$new_configure_args"
fi

###############################################################
# Checks for programs

#c_compilers=notset
#cpp_compilers=notset
#f77_compilers=notset
#fc_compilers=notset

if test $ac_cv_env_empty = yes ; then

  temp_fflags=${FFLAGS}
  temp_fcflags=${FCFLAGS}
  temp_cflags=${CFLAGS}
  temp_cxxflags=${CXXFLAGS}
  
  if test "X${MPI_CC}" = "Xnone" ; then
    if test X${USERDEF_CC} = Xnotset ; then
#      if test X$with_vendor_compilers = Xyes ; then
#        c_compilers="icc pgcc xlc cc gcc"
#      else
#        c_compilers="cc gcc"
#      fi
       c_compilers=notset
    else
      CC="${USERDEF_CC}"
    fi
  else
    CC="${MPI_CC}"
  fi
  
  if test "X${MPI_CXX}" = "Xnone" ; then 
    if test X${USERDEF_CXX} = Xnotset ; then
      if test X${build_os} = Xcygwin ; then
        cpp_compilers="g++"
      else
#        if test X$with_vendor_compilers = Xyes ; then
#          cpp_compilers="icpc pgCC xlC CC c++ g++"
#        else
#          cpp_compilers="c++ g++"
#        fi
         cpp_compilers=notset
      fi
    else
      CXX="${USERDEF_CXX}"
    fi
  else
    CXX="${MPI_CXX}"
  fi
  
  if test "X${MPI_F90}" = "Xnone" ; then 
    if test X${USERDEF_F77} = Xnotset ; then
#      if test X$with_vendor_compilers = Xyes ; then
#        f77_compilers="ifort pgf77 xlf90 gfortran g77 f77 f90"
#      else
#        f77_compilers="gfortran g77 f77 f90 ifort pgf77 xlf90"
#      fi
       f77_compilers=notset
    else
      F77="${USERDEF_F77}"
    fi
  else
    F77="${MPI_F90}"
    FC="${MPI_F90}"
  fi

fi

# Problem: AC_PROG_CC is not the same as AC_PROG_CC([""])

AC_PROG_CC
AC_PROG_CXX
AC_PROG_FC
AC_PROG_F77

if test $ac_cv_env_empty = yes ; then
  # AC_PROG_* macros overwrote our flags
  FFLAGS=${temp_fflags}
  FCFLAGS=${temp_fcflags}
  CFLAGS=${temp_cflags}
  CXXFLAGS=${temp_cxxflags}

fi

if test "X${FC}" = "X" && test "X${F77}" = "X"; then
  ac_cv_have_fortran=no
  scolib_warn="scolib will be built without blas or lapack, because these require fortran"
  gnlp_warn="gnlp will be built without npsol, blas, or lapack, because they require fortran"
  pico_warn="pico will be built without blas or lapack, because they require fortran"
  #
  # appspack has 1 fortran source file, but it looks like it is never built
  #
else
  ac_cv_have_fortran=yes
fi

AC_PROG_LIBTOOL
AC_PROG_CXXCPP
AC_PROG_LN_S
##
## AM_* are automake macros.  Some automake macros are created as
## needed by automake because autoconf has not created them.  When
## an autoconf version is released that defines the following two
## macros, it is important to convert to using the AC_* versions.
##
m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AM_PROG_AS
AM_PROG_CC_C_O

AC_CACHE_SAVE

###############################################################

# Ensure they did not say --enable-shared, which is unsupported
ACRO_NO_SHARED

# # # # # # # # # # # # 
# These two warnings were previously in acro_no_shared.m4, but we
# get an aclocal error when they are there.  I was unabled to figure
# out why, so I moved them into configure.ac.  (It was "diversion"
# related, which has to do with a stack of tasks.  So maybe being
# in an m4 macro called by configure.ac, the stack spilled over?)
#
if test X${ENABLE_SHARED} = Xyes ; then
    AC_MSG_WARN([[--enable-shared: Only libtoolized projects will build a shared library.]])
fi

if test X${ENABLE_STATIC} = Xno ; then
    AC_MSG_WARN([[--disable-static: Know that non-libtoolized projects will only build static libraries in spite of your request.]])
fi
# # # # # # # # # # # # 

# Request executables that are linked only with static libraries,
# default is to use shared when available

ACRO_STATIC_EXECUTABLES

# Ensure they did not say --enable-shared, which is unsupported  lafisk

AC_CACHE_SAVE

###############################################################
# Platform-specific compiler options

# Setup CXX flags
AC_CXX_COMPILER_VENDOR([CXX_VENDOR])

if test $ac_cv_env_empty = yes ; then
  AC_CXX_CPPFLAGS_STD_LANG([CXXFLAGS])
  ACRO_PLATFORM_COMPILER_FLAGS
fi

AM_CONDITIONAL(HAVE_GNU_COMPILER,false)
AM_CONDITIONAL(HAVE_PGI_COMPILER,false)

if test X$ac_cv_cxx_compiler_vendor = Xpgi ; then
  AM_CONDITIONAL(HAVE_PGI_COMPILER,true)
else
  if test X$ac_cv_cxx_compiler_vendor = Xgnu ; then
    AM_CONDITIONAL(HAVE_GNU_COMPILER,true)
  fi
fi

AC_CACHE_SAVE
###############################################################
# Setup compiler flags
#    this macro depends on values previously set in
#       acro_compiler_options
#       acro_platform_compiler_flags

if test $ac_cv_env_empty = yes ; then
  ACRO_COMPILER_FLAGS
fi

###############################################################
# checks for headers

AC_HEADER_STDC

if test "X$ac_cv_header_stdc" = "Xyes"; then
  AM_CONDITIONAL(HAVE_ANSI_C_HEADERS, true)
else
  AM_CONDITIONAL(HAVE_ANSI_C_HEADERS, false)
fi

AC_CHECK_HEADERS([values.h strings.h unistd.h])

AC_CACHE_SAVE
###############################################################
# Checks for libraries

if test "X${ac_cv_have_fortran}" = "Xyes" ; then

  if test $ac_cv_env_empty = yes ; then

    if test "X${FC}" == "X" ; then
      # Find F77 link flags, create FLIBS
      AC_F77_LIBRARY_LDFLAGS
    else
      # Find F90  link flags, create FCLIBS
      AC_FC_LIBRARY_LDFLAGS
    fi

    # Unfortunately when using GCC compilers, -lgcc_s (a 
    # shared-only library) is always added to the link line.
    if test X${ENABLE_STATIC_EXECUTABLES} = Xyes; then
      FLIBS=`echo $FLIBS | sed 's/-lgcc_s//g'`
      FCLIBS=`echo $FCLIBS | sed 's/-lgcc_s//g'`
    fi
  fi

  if test "X${F77}" == "X${FC}" ; then
    FLIBS=${FCLIBS}
  fi

  ACX_BLAS(,[AC_MSG_WARN([No BLAS --- either because it is disabled or could not be found])])

  ACX_LAPACK(,[AC_MSG_WARN([No LAPACK --- either because it is disabled or could not be found])])

  # Define F77_FUNC - used for correctly munging Fortran function calls
  AC_F77_WRAPPERS

  ACX_NPSOLLIB(,[AC_MSG_WARN([No NPSOL --- either because it is disabled or could not be found])])

  ACX_SNOPTLIB
else
  BLAS_LIBS=""
  acx_blas_ok=no
  AC_SUBST(BLAS_LIBS)

  LAPACK_LIBS=""
  acx_lapack_ok=no
  AC_SUBST(LAPACK_LIBS)

  NPSOL_LIBS=""
  acx_npsol_ok=no
  AC_SUBST(NPSOL_LIBS)

  SNOPT_LIBS=""
  acx_snoptlibs_ok=no
  AC_SUBST(SNOPT_LIBS)

fi

ACRO_CPLEX_OPTIONS

AC_CACHE_SAVE
###############################################################
# Validate MPI builds

AM_CONDITIONAL(BUILD_MPI, false)

if test $ac_cv_env_empty = yes ; then

  if test $using_mpi = yes ; then
    #Try compiling with MPI, and set BUILD_MPI to true on success.
    ACRO_MPI_TESTS
  fi
else
  if test $using_mpi = yes ; then
    AM_CONDITIONAL(BUILD_MPI, true)
    AC_DEFINE(HAVE_MPI,,[define that mpi is being used])
  fi
fi

###############################################################
# checks for typedefs, structures and compiler behavior

AC_TYPE_SIZE_T

if test "X${ac_cv_type_size}" = "Xyes" ; then
  AM_CONDITIONAL(HAVE_SIZE_T, true)
else
  AM_CONDITIONAL(HAVE_SIZE_T, false)
fi

AC_CACHE_SAVE

###############################################################
# checks for compiler behavior
#
# AC_CXX_* are checks for C++ behavior
  
# Defines HAVE_NAMESPACES if we can use them
#   TODO - find out if the means std namespace only, or any namespace
#          is it possible we can declare namespaces, but there's no std?
#          Modifying sources to use HAVE_NAMESPACES is not complete
AC_CXX_NAMESPACES

if test X${ac_cv_cxx_namespaces} = Xyes ; then
  AM_CONDITIONAL(HAVE_NAMESPACES, true)
else
  AM_CONDITIONAL(HAVE_NAMESPACES, false)
fi
  
# WEH - turned off because these are not portable
# Defines HAVE_TEMPLATES_AS_TEMPLATE_ARGUMENTS if we can pass a template
# argument in for a template parameter
#AC_CXX_TEMPLATES_AS_TEMPLATE_ARGUMENTS

# Defines HAVE_EXCEPTIONS if we can use throw/catch
AC_CXX_EXCEPTIONS
  
# Defines HAVE_STD if we can include ISO C++ headers
# (iostream, map, iomanip and cmath)
AC_CXX_HAVE_STD

if test X${ac_cv_cxx_have_std} = Xyes ; then
  AM_CONDITIONAL(HAVE_ANSI_CPP_HEADERS, true)
else
  AM_CONDITIONAL(HAVE_ANSI_CPP_HEADERS, false)
fi
  
# Define HAVE_SSTREAM if we have stringstreams
AC_CXX_HAVE_SSTREAM
  
# Define HAVE_EXPLICIT if we can use the explicit keyword
AC_CXX_EXPLICIT
  
# Define HAVE_MEMBER_TEMPLATES if class member functions can be templated
AC_CXX_MEMBER_TEMPLATES

AC_CXX_HAVE_SSTREAM

AC_CACHE_SAVE

###############################################################
# Platform-specific C++ compiler settings

case "$ac_cv_cxx_compiler_vendor" in
    gnu)
        AX_CXXFLAGS_GCC_OPTION([-fpermissive])
    ;;
    compaq)
        AX_CXXFLAGS_OSF_OPTION([-tlocal -distinguish_nested_enums])
    ;;
    sgi)
        AX_CXXFLAGS_IRIX_OPTION([-ptused])
    ;;
    *)
    ;;
esac

AC_CACHE_SAVE

###############################################################
# checks for functions

AC_CHECK_FUNCS([getrusage],
  [AM_CONDITIONAL(HAVE_GETRUSAGE, true)],
  [AM_CONDITIONAL(HAVE_GETRUSAGE, false)])

AC_CHECK_FUNCS([strerror],
  [AM_CONDITIONAL(HAVE_STRERROR, true)],
  [AM_CONDITIONAL(HAVE_STRERROR, false)])

# Find DLOPEN (defines DLOPEN_LIBS to contain the libraries needed to link)
ACX_FUNC_DLOPEN

AC_CACHE_SAVE
###############################################################
# more checks

AC_C_BIGENDIAN()

if test X${ac_cv_c_bigendian} = Xyes ; then
  AM_CONDITIONAL(WORDS_BIGENDIAN, true)
else
  AM_CONDITIONAL(WORDS_BIGENDIAN, false)
fi

AC_CACHE_SAVE

AC_SUBST([abs_top_builddir])

############################################################### 
# We are all done with actions which modify the compilers
# and build flags.  
#

# For mtl, coin-cbc and trilinos
ENV_ARGS="CCAS=\"$CCAS\" CCASFLAGS=\"$CCASFLAGS\" \
      CXXCPP=\"$CXXCPP\" CC=\"$CC\" CFLAGS=\"$CFLAGS\" \
      CPP=\"$CPP\" CPPFLAGS=\"$CPPFLAGS\" \
      CXX=\"$CXX\" CXXFLAGS=\"$CXXFLAGS\" \
      F77=\"$F77\" FFLAGS=\"$FFLAGS\" FLIBS=\"$FLIBS\" \
      FC=\"$FC\" FCFLAGS=\"$FCFLAGS\" FCLIBS=\"$FCLIBS\" \
      LIBS=\"$LIBS\" LDFLAGS=\"$LDFLAGS\""

AC_SUBST([ENV_ARGS])

if test $ac_cv_env_empty = yes ; then

  # For AC_CONFIG_SUBDIR'd packages
  export CCAS
  export CCASFLAGS
  export CXXCPP
  export CC
  export CFLAGS
  export CPP
  export CPPFLAGS
  export CXX
  export CXXFLAGS
  export F77
  export FC
  export FFLAGS
  export FCFLAGS
  export LIBS
  export FLIBS
  export FCLIBS
  export LDFLAGS

  AC_MSG_NOTICE([Adding standard GNU compile environment variables to])
  AC_MSG_NOTICE([the subpackages' configure options.])
fi

if test $use_ac_cv_env = yes && test $ac_cv_env_empty = yes ; then

#  Create the cache variables for the standard GNU compile
#  environment variables.  They don't exist because we specified
#  these with non-standard Acro configure options.

  ac_cv_env_CCASFLAGS_value=$CCASFLAGS
  ac_cv_env_CCASFLAGS_set=set
  ac_cv_env_CCAS_value=$CCAS
  ac_cv_env_CCAS_set=set
  ac_cv_env_CC_value=$CC
  ac_cv_env_CC_set=set
  ac_cv_env_CFLAGS_value=$CFLAGS
  ac_cv_env_CFLAGS_set=set
  ac_cv_env_CPPFLAGS_value=$CPPFLAGS
  ac_cv_env_CPPFLAGS_set=set
  ac_cv_env_CPP_value=$CPP
  ac_cv_env_CPP_set=set
  ac_cv_env_CXXCPP_value=$CXXCPP 
  ac_cv_env_CXXCPP_set=set
  ac_cv_env_CXXFLAGS_value=$CXXFLAGS
  ac_cv_env_CXXFLAGS_set=set
  ac_cv_env_CXX_value=$CXX
  ac_cv_env_CXX_set=set
  ac_cv_env_F77_value=$F77
  ac_cv_env_F77_set=set
  ac_cv_env_FC_value=$FC
  ac_cv_env_FC_set=set
  ac_cv_env_FFLAGS_value=$FFLAGS
  ac_cv_env_FFLAGS_set=set
  ac_cv_env_FCFLAGS_value=$FCFLAGS
  ac_cv_env_FCFLAGS_set=set
  ac_cv_env_LDFLAGS_value=$LDFLAGS
  ac_cv_env_LDFLAGS_set=set
  ac_cv_env_LIBS_value=$LIBS
  ac_cv_env_LIBS_set=set
  ac_cv_env_FLIBS_value=$FLIBS
  ac_cv_env_FLIBS_set=set
  ac_cv_env_FCLIBS_value=$FCLIBS
  ac_cv_env_FCLIBS_set=set

  AC_MSG_NOTICE([Adding standard GNU compile environment variables to])
  AC_MSG_NOTICE([the cache file.])
fi

if test -d userapps ; then
   suffix=$(date +%m.%d.%y.%H.%M.%S%p)
   if test -f userapps/buildFlags.txt ; then
     mv userapps/buildFlags.txt userapps/buildFlags.$suffix
   fi
   
   echo "## " >> userapps/buildFlags.txt
   echo "## Generated $suffix by configure" >> userapps/buildFlags.txt
   echo "## " >> userapps/buildFlags.txt
   echo "CCASFLAGS=$CCASFLAGS" >> userapps/buildFlags.txt
   echo "CCAS=$CCAS" >> userapps/buildFlags.txt
   echo "CC=$CC" >> userapps/buildFlags.txt
   echo "CFLAGS=$CFLAGS" >> userapps/buildFlags.txt
   echo "CPPFLAGS=$CPPFLAGS" >> userapps/buildFlags.txt
   echo "CPP=$CPP" >> userapps/buildFlags.txt
   echo "CXXCPP=$CXXCPP" >> userapps/buildFlags.txt
   echo "CXXFLAGS=$CXXFLAGS" >> userapps/buildFlags.txt
   echo "CXX=$CXX" >> userapps/buildFlags.txt
   echo "F77=$F77" >> userapps/buildFlags.txt
   echo "FC=$FC" >> userapps/buildFlags.txt
   echo "FFLAGS=$FFLAGS" >> userapps/buildFlags.txt
   echo "LDFLAGS=$LDFLAGS" >> userapps/buildFlags.txt
   echo "LIBS=$LIBS" >> userapps/buildFlags.txt
   echo "FLIBS=$FLIBS" >> userapps/buildFlags.txt
   echo "FCLIBS=$FCLIBS" >> userapps/buildFlags.txt
fi

###############################################################
# Platform specific flags for some Acro source files.

ACRO_PLATFORM_CODE_FLAGS

###############################################################
# Options related to including packages and reconfiguring
# autoconf'd subpackages.

# Application Test Dirs
if test -d ${srcdir}/snl-user ; then
      AC_CONFIG_FILES([snl-user/dummy/Makefile:snl-user/dummy/Makefile.in:packages/dummy/Makefile.in])
fi
if test -e ${srcdir}/userapps/Makefile.am ; then
   AC_CONFIG_FILES([userapps/Makefile:userapps/Makefile.in:packages/dummy/Makefile.in])
fi

# sacache
ACRO_ARG_ENABLE_PACKAGE(sacache, SACACHE, sacache,
       ${srcdir}/packages/sacache/Makefile.am, yes)
ACRO_ARG_ENABLE_PACKAGE_RECONFIG(sacache, packages/sacache/config.status, yes)
if test X${ac_cv_reconfigure_sacache} = Xyes ; then
   if test -d ${srcdir}/packages/sacache ; then
      AC_CONFIG_SUBDIRS([packages/sacache])
   fi
fi

# utilib
ACRO_ARG_ENABLE_PACKAGE(utilib, UTILIB, utilib,
       ${srcdir}/packages/utilib/Makefile.am, yes)
ACRO_ARG_ENABLE_PACKAGE_RECONFIG(utilib, packages/utilib/config.status, yes)
if test X${ac_cv_reconfigure_utilib} = Xyes ; then
   if test -d ${srcdir}/packages/utilib ; then
      AC_CONFIG_SUBDIRS([packages/utilib])
   fi
fi

# 3po
ACRO_ARG_ENABLE_PACKAGE(3po, THREEPO, 3po,
       ${srcdir}/tpl/3po/cobyla2c/cobyla.c, yes)
if test X${ac_cv_enable_3po} = Xyes ; then
   if test -d ${srcdir}/tpl/3po ; then
      AC_CONFIG_FILES([tpl/3po/Makefile:tpl/3po/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# NSGA-II
# NB: NSGA2 is built within 3po for convenience, but because we cannot
# distribute it, it needs to be configured separately.
ACRO_ARG_ENABLE_PACKAGE(nsga2, NSGA2, nsga2,
       ${srcdir}/tpl/3po/nsga2/global.h, yes)

# ampl
ACRO_ARG_ENABLE_PACKAGE(ampl, AMPL, ampl,
       ${srcdir}/tpl/ampl/con2val.c, yes)
if test X${ac_cv_enable_ampl} = Xyes ; then
   if test -d ${srcdir}/tpl/ampl ; then
      AC_CONFIG_SUBDIRS([tpl/ampl])
   fi
fi
#if test X${ac_cv_enable_ampl} = Xyes ; then
   #if test -d ${srcdir}/tpl/ampl ; then
      #AC_CONFIG_FILES([tpl/ampl/Makefile:tpl/ampl/Makefile.in:packages/dummy/Makefile.in])
   #fi
#fi

# appspack
ACRO_ARG_ENABLE_PACKAGE(appspack, APPSPACK, appspack,
       ${srcdir}/tpl/appspack/src/APPSPACK_Vector.cpp, yes)
ACRO_ARG_ENABLE_PACKAGE_RECONFIG(appspack, tpl/appspack/config.status, yes)
if test X${ac_cv_reconfigure_appspack} = Xyes ; then
  if test -d ${srcdir}/tpl/appspack ; then
     AC_CONFIG_SUBDIRS([tpl/appspack])
  fi
fi

# coin-bonmin
ACRO_ARG_ENABLE_PACKAGE(coin_bonmin, COIN_BONMIN, coin-bonmin,
       ${srcdir}/tpl/coin-bonmin/Makefile.am, yes)
if test X${ac_cv_enable_coin_bonmin} = Xyes ; then
   if test -d ${srcdir}/tpl/coin-bonmin ; then
      AC_CONFIG_FILES([tpl/coin-bonmin/Makefile:tpl/coin-bonmin/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# coin-cbc
ACRO_ARG_ENABLE_PACKAGE(coin_cbc, COIN_CBC, coin-cbc,
       ${srcdir}/tpl/coin-cbc/Makefile.am, yes)
if test X${ac_cv_enable_coin_cbc} = Xyes ; then
   if test -d ${srcdir}/tpl/coin-cbc ; then
      AC_CONFIG_FILES([tpl/coin-cbc/Makefile:tpl/coin-cbc/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# coin-ipopt
ACRO_ARG_ENABLE_PACKAGE(coin_ipopt, COIN_IPOPT, coin-ipopt,
       ${srcdir}/tpl/coin-ipopt/Makefile.am, no)
if test X${ac_cv_enable_coin_ipopt} = Xyes ; then
   if test -d ${srcdir}/tpl/coin-ipopt ; then
      AC_CONFIG_FILES([tpl/coin-ipopt/Makefile:tpl/coin-ipopt/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# tinyxml
ACX_TINYXML([${srcdir}/tpl/tinyxml],[tpl/tinyxml])
ACRO_ARG_ENABLE_PACKAGE(tinyxml, TINYXML, tinyxml,
       ${srcdir}/tpl/tinyxml/Makefile.am, $acx_tinyxml_ok)
if test X${ac_cv_enable_tinyxml} = Xyes ; then
   if test -d ${srcdir}/tpl/tinyxml ; then
      AC_CONFIG_FILES([tpl/tinyxml/Makefile:tpl/tinyxml/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# conmin
ACRO_ARG_ENABLE_PACKAGE(conmin, CONMIN, conmin,
       ${srcdir}/tpl/conmin/Makefile.am, yes)
if test X${ac_cv_enable_conmin} = Xyes ; then
   if test -d ${srcdir}/tpl/conmin ; then
      AC_CONFIG_FILES([tpl/conmin/Makefile:tpl/conmin/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([tpl/conmin/src/Makefile:tpl/conmin/src/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# dot
ACRO_ARG_ENABLE_PACKAGE(dot, DOT, dot,
       ${srcdir}/tpl/dot/Makefile.am, yes)
if test X${ac_cv_enable_dot} = Xyes ; then
   if test -d ${srcdir}/tpl/dot ; then
      AC_CONFIG_FILES([tpl/dot/Makefile:tpl/dot/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# dscpack
ACRO_ARG_ENABLE_PACKAGE(dscpack, DSCPACK, dscpack,
       ${srcdir}/tpl/dscpack/Makefile.am, yes)
if test X${ENABLE_MPI} = Xyes ; then
   if test X${ac_cv_enable_dscpack} = Xyes ; then
      if test -d ${srcdir}/tpl/dscpack ; then
         AC_CONFIG_FILES([tpl/dscpack/Makefile:tpl/dscpack/Makefile.in:packages/dummy/Makefile.in])
      fi
   fi
else
   DSCPACK_DIR=""
   BUILD_DSCPACK_TRUE='#'
   BUILD_DSCPACK_FALSE=

fi

# eo
ACRO_ARG_ENABLE_PACKAGE(eo, EO, eo,
       ${srcdir}/tpl/eo/Makefile.am, yes)
if test X${ac_cv_enable_eo} = Xyes ; then
   if test -d ${srcdir}/tpl/eo ; then
      AC_CONFIG_FILES([tpl/eo/Makefile:tpl/eo/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# filib
ACRO_ARG_ENABLE_PACKAGE(filib, FILIB, filib,
       ${srcdir}/tpl/filib/filib/bsd/strtod.h, yes)
if test X${ac_cv_enable_filib} = Xyes ; then
   if test -d ${srcdir}/tpl/filib ; then
      AC_CONFIG_FILES([tpl/filib/Makefile:tpl/filib/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# glpk
ACRO_ARG_ENABLE_PACKAGE(glpk, GLPK, glpk,
       ${srcdir}/tpl/glpk/glpk/ChangeLog, yes)
ACRO_ARG_ENABLE_PACKAGE_RECONFIG(glpk,
     tpl/glpk/glpk/config.status, no)
if test X${ac_cv_enable_glpk} = Xyes ; then
   if test -d ${srcdir}/tpl/glpk ; then
      if test X${ac_cv_reconfigure_glpk} = Xyes ; then
        AC_CONFIG_SUBDIRS([tpl/glpk/glpk])
      fi
      AC_CONFIG_FILES([tpl/glpk/Makefile:tpl/glpk/Makefile.in:packages/dummy/Makefile.in])
   fi

    # If libgmp is available, use it when we build glpk
    AC_CHECK_LIB([gmp], [__gmp_randclear], [GMP="yes"], [GMP="no"])
     
    if test $GMP == "yes" ; then
      AC_MSG_NOTICE(["GLPK will use libgmp"])
      AM_CONDITIONAL(USE_GMP_LIB, true)
    else
      AC_MSG_NOTICE(["GLPK will NOT use libgmp, it is not found"])
      AM_CONDITIONAL(USE_GMP_LIB, false)
    fi
else
    AM_CONDITIONAL(USE_GMP_LIB, false)
fi

# momhlib
ACRO_ARG_ENABLE_PACKAGE(momhlib, MOMHLIB, momhlib,
       ${srcdir}/tpl/momhlib/Makefile.am, yes)
if test X${ac_cv_enable_momhlib} = Xyes ; then
   if test -d ${srcdir}/tpl/momhlib ; then
      AC_CONFIG_FILES([tpl/momhlib/Makefile:tpl/momhlib/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# mtl
ACRO_ARG_ENABLE_PACKAGE(mtl, MTL, mtl,
       ${srcdir}/tpl/mtl/mtl/Makefile.am, yes)
  #
  # WEH - reconfiguration doesn't really make sense here
  #
  #AM_CONDITIONAL(RECONFIGURE_MTL, false)
  #ACRO_ARG_ENABLE_PACKAGE_RECONFIG(mtl, tpl/mtl/mtl/config.status, $reconfigure_all)
  #if test X${ac_cv_reconfigure_mtl} = Xyes ; then
  #  AM_CONDITIONAL(RECONFIGURE_MTL, true)
  #fi
  #
  # We don't AC_CONFIG_SUBDIRS because of errors.  We configure
  # instead from the makefile. 
  #
if test X${ac_cv_enable_mtl} = Xyes ; then
   if test -d ${srcdir}/tpl/mtl ; then
      AC_CONFIG_FILES([tpl/mtl/Makefile:tpl/mtl/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# mumps
ACRO_ARG_ENABLE_PACKAGE(mumps, MUMPS, mumps,
       ${srcdir}/tpl/mumps/Makefile.am, no)
if test X${ac_cv_enable_mumps} = Xyes ; then
   if test -d ${srcdir}/tpl/mumps ; then
      AC_CONFIG_FILES([tpl/mumps/Makefile:tpl/mumps/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# npsol
ACRO_ARG_ENABLE_PACKAGE(npsol, NPSOL, npsol,
       ${srcdir}/tpl/npsol/Makefile.am, yes)
if test X${ac_cv_enable_npsol} = Xyes ; then
   if test -d ${srcdir}/tpl/npsol ; then
      AC_CONFIG_FILES([tpl/npsol/Makefile:tpl/npsol/Makefile.in:packages/dummy/Makefile.in])
   fi
fi



# colin
ACRO_ARG_ENABLE_PACKAGE(colin, COLIN, colin,
       ${srcdir}/packages/colin/Makefile.am, yes)
if test X${ac_cv_enable_colin} = Xyes ; then
   if test -d ${srcdir}/packages/colin ; then
      ACRO_BOOST_DEPENDENCE([required])

      AC_CONFIG_FILES([packages/colin/Makefile:packages/colin/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/colin/doc/Makefile:packages/colin/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/colin/test/Makefile:packages/colin/test/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/colin/test/studies/Makefile:packages/colin/test/studies/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/colin/test/driver/Makefile:packages/colin/test/driver/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/colin/test/unit/Makefile:packages/colin/test/unit/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/colin/src/Makefile:packages/colin/src/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/colin/src/colin/Makefile:packages/colin/src/colin/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# scolib
ACRO_ARG_ENABLE_PACKAGE(scolib, SCOLIB, scolib,
       ${srcdir}/packages/scolib/Makefile.am, yes)
if test X${ac_cv_enable_scolib} = Xyes ; then
   if test -d ${srcdir}/packages/scolib ; then
      AC_CONFIG_FILES([packages/scolib/Makefile:packages/scolib/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/scolib/doc/Makefile:packages/scolib/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/scolib/src/Makefile:packages/scolib/src/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/scolib/test/Makefile:packages/scolib/test/Makefile.in:packages/dummy/Makefile.in])
   fi
   if test "X${ac_cv_have_fortran}"="Xno" ; then
     AC_MSG_WARN([$scolib_warn])
   fi
fi

# gnlp
ACRO_ARG_ENABLE_PACKAGE(gnlp, GNLP, gnlp,
       ${srcdir}/packages/gnlp/src/gnlp/gnlpdag.cpp, yes)
if test X${ac_cv_enable_gnlp} = Xyes ; then
   if test -d ${srcdir}/packages/gnlp ; then
      AC_CONFIG_FILES([packages/gnlp/Makefile:packages/gnlp/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/gnlp/doc/Makefile:packages/gnlp/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/gnlp/src/Makefile:packages/gnlp/src/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/gnlp/src/headers/Makefile:packages/gnlp/src/headers/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/gnlp/test/Makefile:packages/gnlp/test/Makefile.in:packages/dummy/Makefile.in])
   fi
   if test "X${ac_cv_have_fortran}"="Xno" ; then
     AC_MSG_WARN([$gnlp_warn])
   fi
fi

# hsl
ACRO_ARG_ENABLE_PACKAGE(hsl, HSL, hsl,
       ${srcdir}/tpl/hsl/Makefile.am, yes)
if test X${ac_cv_enable_hsl} = Xyes ; then
   if test -d ${srcdir}/tpl/hsl ; then
      AC_CONFIG_FILES([tpl/hsl/Makefile:tpl/hsl/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# interfaces
ACRO_ARG_ENABLE_PACKAGE(interfaces, INTERFACES, interfaces,
       ${srcdir}/packages/interfaces/Makefile.am, yes)
if test X${ac_cv_enable_interfaces} = Xyes ; then
   if test -d ${srcdir}/packages/interfaces ; then
      AC_CONFIG_FILES([packages/interfaces/Makefile:packages/interfaces/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/interfaces/doc/Makefile:packages/interfaces/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/interfaces/test/Makefile:packages/interfaces/test/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/interfaces/src/Makefile:packages/interfaces/src/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# jega
ACRO_ARG_ENABLE_PACKAGE(jega, JEGA, jega,
       ${srcdir}/packages/jega/Makefile.am, yes)
ACRO_ARG_ENABLE_PACKAGE_RECONFIG(jega, packages/jega/config.status, yes)
if test X${ac_cv_reconfigure_jega} = Xyes ; then
   if test -d ${srcdir}/packages/jega ; then
      AC_CONFIG_SUBDIRS([packages/jega])
   fi
fi


# OPT++
if test X$ac_cv_cxx_compiler_vendor = Xpgi ; then 
  AC_MSG_NOTICE([Omitting Opt++ build because it is not yet ported to PGI])
  AM_CONDITIONAL([BUILD_TEUCHOS], false)
  AM_CONDITIONAL(BUILD_OPTPP,false)
else
  ACRO_ARG_ENABLE_PACKAGE(optpp, OPTPP, optpp,
         ${srcdir}/packages/optpp/Makefile.am, yes)
  ACRO_ARG_ENABLE_PACKAGE_RECONFIG(optpp, packages/optpp/config.status, yes)
  if test X${ac_cv_reconfigure_optpp} = Xyes ; then
     if test -d ${srcdir}/packages/optpp ; then
        dnl OPT++ requires teuchos, but is possibly in another project
	dnl determine whether to build the local teuchos
	dnl this macro will set BUILD_TEUCHOS and TEUCHOS_DIR if needed
	ACRO_AC_TEUCHOS()
        AC_CONFIG_SUBDIRS([packages/optpp])
     else
        AM_CONDITIONAL([BUILD_TEUCHOS], false)
     fi
  else
    AM_CONDITIONAL([BUILD_TEUCHOS], false)
  fi
fi

# parpcx
ACRO_ARG_ENABLE_PACKAGE(parpcx, PARPCX, parpcx,
       ${srcdir}/packages/parpcx/src/main/io.cc, no)
if test X${ac_cv_enable_parpcx} = Xyes ; then
   if test -d ${srcdir}/packages/parpcx ; then
      AC_CONFIG_FILES([packages/parpcx/Makefile:packages/parpcx/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/parpcx/src/Makefile:packages/parpcx/src/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/parpcx/doc/Makefile:packages/parpcx/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/parpcx/src/headers/Makefile:packages/parpcx/src/headers/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/parpcx/src/pcx/Makefile:packages/parpcx/src/pcx/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/parpcx/test/Makefile:packages/parpcx/test/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# pebbl
ACRO_ARG_ENABLE_PACKAGE(pebbl, PEBBL, pebbl,
       ${srcdir}/packages/pebbl/src/bb/pebbl/branching.h, yes)
if test X${ac_cv_enable_pebbl} = Xyes ; then
   if test -d ${srcdir}/packages/pebbl ; then
      AC_CONFIG_FILES([packages/pebbl/Makefile:packages/pebbl/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pebbl/doc/Makefile:packages/pebbl/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pebbl/src/Makefile:packages/pebbl/src/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pebbl/src/headers/Makefile:packages/pebbl/src/headers/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pebbl/test/Makefile:packages/pebbl/test/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pebbl/test/unit/Makefile:packages/pebbl/test/unit/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# pico
AM_CONDITIONAL(PICO_TEST_DIR_COMPILES, false)
ACRO_ARG_ENABLE_PACKAGE(pico, PICO, pico,
       ${srcdir}/packages/pico/src/milp/pico/parMilp.h, yes)
if test X${ac_cv_enable_pico} = Xyes ; then
   if test -d ${srcdir}/packages/pico ; then
      AC_CONFIG_FILES([packages/pico/Makefile:packages/pico/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pico/doc/Makefile:packages/pico/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pico/src/Makefile:packages/pico/src/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pico/src/headers/Makefile:packages/pico/src/headers/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pico/test/Makefile:packages/pico/test/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/pico/test/unit/Makefile:packages/pico/test/unit/Makefile.in:packages/dummy/Makefile.in])
   fi
   if test "X${ac_cv_have_fortran}"="Xno" ; then
     AC_MSG_WARN([$pico_warn])
   fi
fi

# snopt
ACRO_ARG_ENABLE_PACKAGE(snopt, SNOPT, snopt,
       ${srcdir}/tpl/snopt/Makefile.am, yes)
if test X${ac_cv_enable_snopt} = Xyes ; then
   if test -d ${srcdir}/tpl/snopt ; then
      AC_CONFIG_FILES([tpl/snopt/Makefile:tpl/snopt/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# soplex
ACRO_ARG_ENABLE_PACKAGE(soplex, SOPLEX, soplex,
       ${srcdir}/tpl/soplex/SOPLEX/README, no)
if test X${ac_cv_enable_soplex} = Xyes ; then
   if test -d ${srcdir}/tpl/soplex ; then
      AC_CONFIG_FILES([tpl/soplex/Makefile:tpl/soplex/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# sundry
ACRO_ARG_ENABLE_PACKAGE(sundry, SUNDRY, sundry,
       ${srcdir}/packages/sundry/sundry/sundry.h, yes)
if test X${ac_cv_enable_sundry} = Xyes ; then
   if test -d ${srcdir}/packages/sundry ; then
      AC_CONFIG_FILES([packages/sundry/Makefile:packages/sundry/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/sundry/test/Makefile:packages/sundry/test/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/sundry/sundry/Makefile:packages/sundry/sundry/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/sundry/src/Makefile:packages/sundry/src/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# tmf
ACRO_ARG_ENABLE_PACKAGE(tmf, TMF, tmf,
       ${srcdir}/packages/tmf/src/dummy.cpp, yes)
if test X${ac_cv_enable_tmf} = Xyes ; then
   if test -d ${srcdir}/packages/tmf ; then
      AC_CONFIG_FILES([packages/tmf/Makefile:packages/tmf/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/tmf/doc/Makefile:packages/tmf/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/tmf/src/Makefile:packages/tmf/src/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/tmf/src/headers/Makefile:packages/tmf/src/headers/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/tmf/test/Makefile:packages/tmf/test/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# tracecache
ACRO_ARG_ENABLE_PACKAGE(tracecache, TRACECACHE, tracecache,
       ${srcdir}/packages/tracecache/src/dummy.cpp, yes)
if test X${ac_cv_enable_tracecache} = Xyes ; then
   if test -d ${srcdir}/packages/tracecache ; then
      AC_CONFIG_FILES([packages/tracecache/Makefile:packages/tracecache/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/tracecache/doc/Makefile:packages/tracecache/doc/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/tracecache/src/Makefile:packages/tracecache/src/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/tracecache/src/headers/Makefile:packages/tracecache/src/headers/Makefile.in:packages/dummy/Makefile.in])
      AC_CONFIG_FILES([packages/tracecache/test/Makefile:packages/tracecache/test/Makefile.in:packages/dummy/Makefile.in])
   fi
fi

# trilinos
ACRO_ARG_ENABLE_PACKAGE(trilinos, TRILINOS, trilinos,
       ${srcdir}/tpl/trilinos/trilinos/CHANGELOG, yes)
if test X${ac_cv_enable_trilinos} = Xyes ; then
   if test -d ${srcdir}/tpl/trilinos ; then
      AC_CONFIG_FILES([tpl/trilinos/Makefile:tpl/trilinos/Makefile.in:packages/dummy/Makefile.in])
      #AC_CONFIG_SUBDIRS([tpl/trilinos/trilinos])
   fi
fi

AC_CONFIG_FILES([packages/dummy/Makefile])

# ------------------------------------------------------------------------
# Other package options
# ------------------------------------------------------------------------

AX_ENABLE_R()

#
# --enable-clp=yes is the default
#
AC_ARG_ENABLE(clp,
AS_HELP_STRING([--disable-clp],[Turn off the CLP LP solver]),
[ENABLE_CLP=$enableval],
[ENABLE_CLP=yes]
) 

if test X${ENABLE_CLP} = Xyes ; then
  AC_DEFINE(USING_CLP,1,[define that clp is being used])
fi

#
## TODO: are we using this anywhere?
# --enable-cobyla=yes is the default
#
AC_ARG_ENABLE(cobyla,
AS_HELP_STRING([--disable-cobyla],[Turn off the Cobyla solver]),
[ENABLE_COBYLA=$enableval],
[ENABLE_COBYLA=yes]
) 

if test X${ENABLE_COBYLA} = Xyes ; then
  AC_DEFINE(USING_COBYLA,1,[define that cobyla is being used])
fi

#
# --enable-plgo=yes is the default
#
AC_ARG_ENABLE(plgo,
AS_HELP_STRING([--disable-plgo],[Turn off the PLGO solver]),
[ENABLE_PLGO=$enableval],
[ENABLE_PLGO=yes]
) 

if test X${ENABLE_PLGO} = Xyes ; then
  AC_DEFINE(USING_PLGO,1,[define that plgo is being used])
fi

AC_MSG_CHECKING([whether to build PICO with GLPK support])

AC_ARG_WITH(glpk-in-pico,
      [AS_HELP_STRING([--without-glpk-in-pico],[Do not link GLPK into the PICO executable])],
      [WITH_GLPK_PICO=$withval],
      [WITH_GLPK_PICO=$ac_cv_enable_pico])

if test X${WITH_GLPK_PICO} = Xyes ; then
  AC_DEFINE([USING_GLPK_IN_PICO],1,[Define if we want to use GLPK in PICO])
  if test X${ac_cv_enable_glpk} = Xno ; then
    AC_MSG_ERROR("--with-glpk-in-pico=yes but --enable-glpk=no")
  fi
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi



#
# --with-soplex-1-2-1 will build the old version of soplex instead
#
#AC_ARG_WITH(soplex-1-2-1,
#AS_HELP_STRING([--with-soplex-1-2-1],[Build this older revision of soplex instead of the current one]),
#[WITH_OLD_SOPLEX=$withval],
#[WITH_OLD_SOPLEX=no]
#) 

#AM_CONDITIONAL(BUILD_SOPLEX, true)
#if test X${WITH_OLD_SOPLEX} = Xyes ; then
#  AM_CONDITIONAL(BUILD_SOPLEX, true)
#else
#  AM_CONDITIONAL(BUILD_SOPLEX, false)
#fi

#AM_CONDITIONAL(BUILD_COIN_20050626, false)
#AM_CONDITIONAL(BUILD_COIN_20060202, false)

###############################################################
# Boost configuration - we do this last so that optional packages may
# "up" the Boost dependence requirement
AC_MSG_NOTICE([boost dependence: [[$acro_boost_dependence]]])
AX_BOOST_BASE([1.37], [$acro_boost_dependence], [$srcdir/tpl/boost], [tpl/boost])
if test "$ac_boost_build_tpl" = "yes"; then
	AC_MSG_NOTICE([will build bundled boost TPL.])
	AC_CONFIG_SUBDIRS([tpl/boost])
else
	AC_MSG_NOTICE([skipping bundled boost TPL.])
fi

###############################################################
# Output of autoconf/automake

AC_CONFIG_FILES([Makefile] [doc/Makefile] [packages/Makefile] [tpl/Makefile])

#
# If we are configuring from a cache file, TRILINOS and APPSPACK
# both need the original "--with-mpi*=" options or they will
# not know they are building with MPI.  Add these options now.
#
if test $use_ac_cv_env = yes && test "X$acro_cv_with_mpi_set" = "Xyes" ; then
  ac_configure_args="$acro_cv_with_mpi_options $ac_configure_args"
fi

AC_ARG_WITH(python,
AS_HELP_STRING([--without-python],[Skip creation of python environment]),
[DO_PYINSTALL=$withval],
[DO_PYINSTALL=yes])

if test X${DO_PYINSTALL} = Xyes; then
  AM_CONDITIONAL(DO_PYINSTALL, true)
else
  AM_CONDITIONAL(DO_PYINSTALL, false)
fi

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # #

AC_OUTPUT

if test $use_ac_cv_env = no && test $cache_file != "/dev/null" ; then
  AC_MSG_WARN([ ============== COMPILE ENVIRONMENT ============== ])
fi

AC_MSG_NOTICE(CCAS=\"$CCAS\" CCASFLAGS=\"$CCASFLAGS\")
AC_MSG_NOTICE(CXXCPP=\"$CXXCPP\" CC=\"$CC\" CFLAGS=\"$CFLAGS\")
AC_MSG_NOTICE(CPP=\"$CPP\" CPPFLAGS=\"$CPPFLAGS\") 
AC_MSG_NOTICE(CXX=\"$CXX\" CXXFLAGS=\"$CXXFLAGS\")
AC_MSG_NOTICE(F77=\"$F77\" FC=\"$FC\" FFLAGS=\"$FFLAGS\" )
AC_MSG_NOTICE(FLIBS=\"$FLIBS\" )
AC_MSG_NOTICE(FCLIBS=\"$FCLIBS\" )
AC_MSG_NOTICE(LDFLAGS=\"$LDFLAGS\" LIBS=\"$LIBS\")

if test $use_ac_cv_env = no && test $cache_file != "/dev/null" ; then
  AC_MSG_WARN([ ================================================= ])
  AC_MSG_WARN([ In subsequent configure runs that use the "-C" or ])
  AC_MSG_WARN([ "--cache-file=" options, you must provide the ])
  AC_MSG_WARN([ compile environment on the configure command line ])
  AC_MSG_WARN([ using the above flags.  The Acro compile environment ])
  AC_MSG_WARN([ options (like "--with-debugging" or "--with-cc=") are not ])
  AC_MSG_WARN([ compatible with GNU autoconf's cache variable feature.])
  AC_MSG_WARN([ ================================================= ])
fi


echo ""
echo "---"
echo "--- Thank you for downloading Acro. For more information"
echo "--- visit:  http://software.sandia.gov/Acro/"
echo "---"
echo ""

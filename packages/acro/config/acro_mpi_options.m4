dnl ACRO MPI options

AC_DEFUN([ACRO_MPI_OPTIONS],[
  dnl
  dnl Command-line options for setting up MPI
  dnl 

MPI_CXX=none
MPI_CC=none
MPI_F90=none
MPI_F77=none

###
# If TRILINOS is being built as a sub-package, we need to cache these
# variables (if a cache file is being used).  When configuring
# from a cache file, the "--with-mpi-*=" command line options are not
# used.  Instead, the CC, CFLAGS, and so on variables are properly
# set to build with MPI.  But TRILINOS does not know it is being built
# with MPI unless these command line options are present.  So in the
# case of configuration from a cache file, we will add these on the
# TRILINOS configure command line.
# 
acro_cv_with_mpi_set=no
acro_cv_with_mpi_options=""
###

AC_ARG_WITH(mpi-compilers,
AS_HELP_STRING([--with-mpi-compilers=DIR or --with-mpi-compilers=yes],
[use MPI compilers (mpicc, etc.) found in directory DIR, or in your PATH if =yes]),
[
  ENABLE_MPI=yes

  if test "X${withval}" = "Xyes"; then
    # Look for MPI C++ compile script/program in user's path, set
    # MPI_CXX to name of program found, else set it to "none".  Could
    # add more paths to check as 4th argument if you wanted to.
    # Calls AC_SUBST for MPI_CXX, etc.

    if test "X${MPI_CXX}" = "Xnone" ; then 
      echo "Looking for an MPI C++ compiler in your path"
      AC_CHECK_PROGS([MY_CXX], [mpic++ mpicxx mpiCC mpiicpc], [none],)
      MPI_CXX=${MY_CXX}
    fi
    if test "X${MPI_CC}" = "Xnone" ; then 
      echo "Looking for an MPI C compiler in your path"
      AC_CHECK_PROGS([MY_CC], [mpicc mpiicc], [none],)
      MPI_CC=${MY_CC}
    fi
    if test "X${MPI_F90}" = "Xnone" ; then 
      echo "Looking for an optional MPI F90 compiler in your path, or f77"
      AC_CHECK_PROGS([MY_F90], [mpif90 mpiifort mpif77], [none],)
      MPI_F90=${MY_F90}
    fi

  else
    foundCompiler=no
    if test "X${MPI_CXX}" = "Xnone" ; then
      AC_MSG_CHECKING(MPI C++ compiler in ${withval})
      if test -f ${withval}/mpic++ ; then
        MPI_CXX=${withval}/mpic++
        foundCompiler=yes
      fi
      if test "X${MPI_CXX}" = "Xnone" && test -f ${withval}/mpicxx ; then
        MPI_CXX=${withval}/mpicxx
        foundCompiler=yes
      fi
      if test "X${MPI_CXX}" = "Xnone" && test -f ${withval}/mpiCC ; then
        MPI_CXX=${withval}/mpiCC
        foundCompiler=yes
      fi
      if test "X${MPI_CXX}" = "Xnone" && test -f ${withval}/mpiicpc ; then
        MPI_CXX=${withval}/mpiicpc
        foundCompiler=yes
      fi
      AC_MSG_RESULT([${MPI_CXX}])
    fi

    if test "X${MPI_CC}" = "Xnone" ; then
      AC_MSG_CHECKING(MPI C compiler in ${withval})
      if test -f ${withval}/mpicc ; then
        MPI_CC=${withval}/mpicc
        foundCompiler=yes
      fi
      if test "X${MPI_CC}" = "Xnone" && test -f ${withval}/mpiicc ; then
        MPI_CC=${withval}/mpiicc
        foundCompiler=yes
      fi
      AC_MSG_RESULT([${MPI_CC}])
    fi

    if test "X${MPI_F90}" = "Xnone" ; then
      AC_MSG_CHECKING(optional MPI Fortran compiler in ${withval})
      if test -f ${withval}/mpif90 ; then
        MPI_F90=${withval}/mpif90
        foundCompiler=yes
      fi
      if test "X${MPI_F90}" = "Xnone" && test -f ${withval}/mpiifort ; then
        MPI_F90=${withval}/mpiifort
        foundCompiler=yes
      fi
      if test "X${MPI_F90}" = "Xnone" && test -f ${withval}/mpif77; then
        MPI_F90=${withval}/mpif77
        foundCompiler=yes
      fi
      AC_MSG_RESULT([${MPI_F90}])
    fi

#    if test "$foundCompiler" = "yes" ; then
#      PATH=${withval}${PATH_SEPARATOR}${PATH}
#    fi
  fi

  if test "X${MPI_CXX}" = "Xnone" ; then
    AC_MSG_ERROR([MPI C++ compiler script/program not found.])
  fi

  if test "X${MPI_CC}" = "Xnone" ; then
    AC_MSG_ERROR([MPI C compiler script/program (mpicc) not found.])
  fi

  # Check that C++ and C compilers are from same distribution, if there
  # is a path component to the name

  cxx_path=${MPI_CXX%/*}
  cc_path=${MPI_CC%/*}

  if ( test "${cxx_path}" = "${MPI_CXX}" ) && ( test "${cc_path}" = "${MPI_CC}" ) ; then : ; else
    if test "${cxx_path}" = "${cc_path}" ; then : ; else
      AC_MSG_WARN([Found ${MPI_CXX} and ${MPI_CC}.])
      AC_MSG_ERROR([Use --with-mpi-compilers=${cxx_path} or --with-mpi-compilers=${cc_path}])
    fi
  fi

  if test "X${MPI_F90}" = "Xnone" ; then : ; else
    f90_path=${MPI_F90%/*}
    if test "${f90_path}" = "${MPI_F90}" ; then : ; else
      if test "${cxx_path}" = "${f90_path}" ; then : ; else
        AC_MSG_WARN([Found ${MPI_CXX} and ${MPI_F90}.])
        AC_MSG_ERROR([Use --with-mpi-compilers=${cxx_path} or --with-mpi-compilers=${f90_path}])
      fi
    fi
  fi

  acro_cv_with_mpi_set=yes
  ACRO_ADD(acro_cv_with_mpi_options, --with-mpi-compilers=${withval})
]
)

if test "X${MPI_CXX}" = "Xnone" ; then
  if test "X${MPI_CC}" = "Xnone" ; then : ; else
    AC_MSG_ERROR([No C++ compiler: try --with-mpi-compilers= or --with-mpi-cxx=])
  fi
else
  if test "X${MPI_CC}" = "Xnone" ; then
    AC_MSG_ERROR([No C compiler: try --with-mpi-compilers= or --with-mpi-cc=])
  fi
fi


AC_ARG_WITH(mpi,
AS_HELP_STRING([--with-mpi=MPIROOT],[use MPI root directory. Automatically enables MPI.]),
[
  # This if statement is necessary (for now) to make this compatible with DAKOTA
  if test "X${withval}" = "Xno"; then :; else
     ENABLE_MPI=yes
     MPI_DIR=${withval}
     AC_MSG_CHECKING(MPI directory)
     AC_MSG_RESULT([${MPI_DIR}])

     acro_cv_with_mpi_set=yes
     ACRO_ADD(acro_cv_with_mpi_options, --with-mpi=${withval})
  fi
]
)

AC_ARG_WITH(mpi-libs,
AS_HELP_STRING([--with-mpi-libs="LIBS"],[MPI libraries @<:@default "-lmpi"@:>@]),
[
  if test "X${withval}" = "Xno"; then :; else
    ENABLE_MPI=yes
    MPI_LIBS=${withval}
    AC_MSG_CHECKING(user-defined MPI libraries)
    AC_MSG_RESULT([${MPI_LIBS}])

    acro_cv_with_mpi_set=yes
    ACRO_ADD(acro_cv_with_mpi_options, --with-mpi-libs=\"$MPI_LIBS\")
  fi
]
)

AC_ARG_WITH(mpi-incdir,
AS_HELP_STRING([--with-mpi-incdir=DIR],[MPI include directory @<:@default MPIROOT/include@:>@]),
[
  if test "X${withval}" = "Xno"; then :; else
    ENABLE_MPI=yes
    MPI_INC=${withval}
    AC_MSG_CHECKING(user-defined MPI includes)
    AC_MSG_RESULT([${MPI_INC}])

    acro_cv_with_mpi_set=yes
    ACRO_ADD(acro_cv_with_mpi_options, --with-mpi-incdir=${withval})
  fi
]
)

AC_ARG_WITH(mpi-libdir,
AS_HELP_STRING([--with-mpi-libdir=DIR],[MPI library directory @<:@default MPIROOT/lib@:>@]),
[
  if test "X${withval}" = "Xno"; then :; else
    ENABLE_MPI=yes
    MPI_LIBDIR=${withval}
    AC_MSG_CHECKING(user-defined MPI library directory)
    AC_MSG_RESULT([${MPI_LIBDIR}])

    acro_cv_with_mpi_set=yes
    ACRO_ADD(acro_cv_with_mpi_options, --with-mpi-libdir=${withval})
  fi
]
)

AC_ARG_WITH(mpi-cppflags,
AS_HELP_STRING([--with-mpi-cppflags=flags],[Extra preprocessor flags for MPI]),
[
  if test "X${withval}" = "Xno"; then :; else
    ENABLE_MPI=yes
    MPI_CPPFLAGS=${withval}
    AC_MSG_CHECKING(user-defined MPI preprocessor flags)
    AC_MSG_RESULT([${MPI_CPPFLAGS}])
  fi
]
)

AC_ARG_WITH(mpi-ldflags,
AS_HELP_STRING([--with-mpi-ldflags=flags],[Extra link flags for MPI]),
[
  if test "X${withval}" = "Xno"; then :; else
    ENABLE_MPI=yes
    MPI_LDFLAGS=${withval}
    AC_MSG_CHECKING(user-defined MPI link flags)
    AC_MSG_RESULT([${MPI_LDFLAGS}])
  fi
]
)

AC_MSG_CHECKING(whether we are using MPI)
AC_MSG_RESULT([${ENABLE_MPI}])

if test "X${ENABLE_MPI}" = "Xyes" ; then
  using_acro_special=yes
fi



])

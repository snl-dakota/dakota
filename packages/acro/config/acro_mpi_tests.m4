dnl ACRO MPI tests
dnl
dnl   ACRO_MPI_TESTS(subpackage)
dnl
dnl     Use "subpackage" option to omit tests and just set
dnl     makefile conditionals and variables.  Also avoids
dnl     repeating MPI paths in CPPFLAGS and LDFLAGS.

AC_DEFUN([ACRO_MPI_TESTS],[
  dnl
  dnl Test to see if MPI compilers work properly
  dnl 

dnl
dnl Preprocess and compile check
dnl
if test "X${ENABLE_MPI}" = "Xyes"; then

  if test -n "${MPI_DIR}" && test -z "${MPI_INC}"; then
    MPI_INC="${MPI_DIR}/include"
  fi

  if test X$1 = Xsubpackage ; then
      AC_DEFINE(HAVE_MPI,,[define that mpi is being used])
  else
    if test "X${MPI_CXX}" = "Xnone" || test "X${MPI_CC}" = "Xnone" ; then
      ACRO_ADD(CPPFLAGS, -I${MPI_INC})
    fi
  
    if test -n "${MPI_CPPFLAGS}" ; then
      ACRO_ADD(CPPFLAGS, ${MPI_CPPFLAGS})
    fi
  
    AC_LANG([C++])
    AC_MSG_CHECKING(whether we can preprocess mpi.h)
    AC_PREPROC_IFELSE(
    [AC_LANG_SOURCE([[#include "mpi.h"]])],
    [
      AC_MSG_RESULT(yes)
    ],[
      AC_MSG_RESULT(no)
      echo "---"
      echo "Cannot find header file mpi.h."
      echo "Either compile without mpi, or view the mpi options with \"configure --help\"."
      echo "---"
      AC_MSG_ERROR(cannot find mpi.h)
    ])

    AC_MSG_CHECKING(whether we can compile mpi.h)
    AC_COMPILE_IFELSE(
    [AC_LANG_SOURCE([[#include "mpi.h"]],[[int c; char** v; MPI_Init(&c,&v);]])],
    [
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_MPI,,[define that mpi is being used])
    ],[
      AC_MSG_RESULT(no)
      echo "---"
      echo "mpi.h has compile errors"
      echo "View the mpi options with \"configure --help\", and provide a valid MPI."
      echo "---"
      AC_MSG_ERROR(invalid mpi.h)
    ])
  fi

  AM_CONDITIONAL(BUILD_MPI, true)
  acro_cv_enable_mpi=yes

  dnl We only require F77, but finding the right libraries at link time
  dnl is more robust if we choose F90 when it is available

  MPI_F77=${MPI_F90}

  AC_SUBST([MPI_INC])
  AC_SUBST([MPI_CXX])
  AC_SUBST([MPI_CC])
  AC_SUBST([MPI_F90])
  AC_SUBST([MPI_F77])

else

  AM_CONDITIONAL(BUILD_MPI, false)
  acro_cv_enable_mpi=no

fi

dnl
dnl Link check
dnl


if test "X${ENABLE_MPI}" = "Xyes"; then
  if test "X${ENABLE_TESTS}" = "Xyes" || test "X${ENABLE_EXECUTABLES}" = "Xyes"; then
  
    if test -n "${MPI_DIR}" && test -z "${MPI_LIBDIR}"; then
      MPI_LIBDIR="${MPI_DIR}/lib"
    fi
  
    if test -z "${MPI_LIBS}" && test -n "${MPI_LIBDIR}"; then
      MPI_LIBS="-lmpi"
    fi
  
    if test X$1 = Xsubpackage ; then :; else
      if test -n "${MPI_LDFLAGS}" ; then
        ACRO_ADD(LDFLAGS, ${MPI_LDFLAGS})
      fi
    
      AC_LANG([C++])
    
      if test "X${MPI_CXX}" = "Xnone" || test "X${MPI_CC}" = "Xnone" ; then
        ACRO_ADD(LIBS, ${MPI_LIBS})
        ACRO_ADD(LDFLAGS, -L${MPI_LIBDIR})
      fi

      AC_MSG_CHECKING(whether special compile flag for MPICH is required)
      AC_RUN_IFELSE(
      [AC_LANG_PROGRAM(
             [[#define MPICH_IGNORE_CXX_SEEK]
              [#include <mpi.h>]], 
             [[#ifdef MPICH_NAME
                 return 0; 
               #endif
               return 1;]])],
      [AC_MSG_RESULT(yes)
       ACRO_ADD(CPPFLAGS, -DMPICH_IGNORE_CXX_SEEK)
       echo "-----"
       echo "Adding -DMPICH_IGNORE_CXX_SEEK to MPICH compilations"
       echo "-----"],
      [AC_MSG_RESULT(no)],
      [AC_MSG_RESULT(cross compiling)
       ACRO_ADD(CPPFLAGS, -DMPICH_IGNORE_CXX_SEEK)
       echo "-----"
       echo "Adding -DMPICH_IGNORE_CXX_SEEK because we can't determine whether or not it is required"
       echo "-----"]
      )
    
      AC_MSG_CHECKING(whether MPI will link using C++ compiler)
      AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[#include <mpi.h>]], [[int c; char** v; MPI_Init(&c,&v);]])],
      [AC_MSG_RESULT(yes)],
      [AC_MSG_RESULT(no)
       echo "-----"
       echo "Cannot link simple MPI program."
       echo "Either compile without mpi,"
       echo "or view the mpi options with \"configure --help\" and re-configure."
       echo "-----"
       AC_MSG_ERROR(MPI cannot link)
      ])
    fi
  fi
fi

])

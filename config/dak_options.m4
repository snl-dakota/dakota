dnl DAKOTA Options

AC_DEFUN([DAK_OPTIONS],[
  dnl Debug option check.
  AC_ARG_ENABLE([debug],AS_HELP_STRING([--enable-debug],
				       [turn debug support on]),
	  	[enable_debug=$enableval],[enable_debug=no])
  if test "x$enable_debug" = xyes; then
     AC_DEFINE([HAVE_DAKOTA_DEBUG],[1],[Macro to enable debugging in Dakota.])
     AC_DEFINE([HAVE_DAKOTA_BOUNDSCHECK],[1],
		[Macro to enable boundschecking in Dakota.])
  else
     AC_DEFINE([BOOST_DISABLE_ASSERTS],[1],[Macro to disable Boost asserts.])
  fi

  dnl Tests option check.
  AC_ARG_ENABLE([tests],AS_HELP_STRING([--enable-tests],
				       [turn testing on]),
	  	[enable_tests=$enableval],[enable_tests=yes])
  AM_CONDITIONAL([ENABLE_TESTS],[test "x$enable_tests" = xyes])

  dnl Documentation option check.
  AC_ARG_ENABLE([docs],AS_HELP_STRING([--enable-docs],
				      [turn documentation support on]),
	  	[enable_docs=$enableval],[enable_docs=no])
  if test "x$enable_docs" = xyes; then
    AC_CONFIG_SUBDIRS([docs])
  fi
  AM_CONDITIONAL([ENABLE_DOCS],[test "x$enable_docs" = xyes])

  dnl MPI option checks.
  AC_ARG_ENABLE([mpi],AS_HELP_STRING([--disable-mpi],[turn MPI support off]),
		[enable_mpi=$enableval],[enable_mpi=yes])
  if test "x$enable_mpi" = xyes; then
    dnl MPI preprocessor defines and exports.
    ACX_MPI
    if test "$MPICXX" = "$CXX" -a -z "$MPILIBS"; then
      AC_MSG_ERROR([Cannot find a MPI compiler wrapper; please alter your PATH
		  or specify the absolute path to the compiler wrapper using
		  the MPICXX environment variable when running configure.
		  Alternatively, run configure with the --disable-mpi flag.])
    else
      AC_DEFINE([USE_MPI],[1],[Macro to handle code which depends on MPI.])
      AC_DEFINE([USING_MPI],[1],[Macro to handle code which depends on MPI.])
      CXX="$MPICXX"
      CXXLINK="$MPICXX"
      LIBS="$MPILIBS $LIBS"
    fi
    dnl Platform-specific MPI preprocessor defines.
    case "${host_os}" in
	*linux*)
		AC_CHECK_LIB([mpich],[MPI_Get_version],
		AC_DEFINE([HAVE_MPICH],[1],[MPICH MPI identifier.]))dnl MPICH
		;;
	*solaris*)
		AC_CHECK_LIB([mpi],[MPI_Get_version],
			     AC_DEFINE([HAVE_SOLARIS64_MPI],[1],
			  	       [Solaris 64-bit MPI identifier.]),
			     AC_CHECK_LIB([mpich],[PMPI_Get_version],
			     AC_DEFINE([HAVE_MPICH],[1],
				       [MPICH MPI identifier.]))dnl MPICH
		)dnl Solaris 64
		;;
	*aix*)
		AC_DEFINE([HAVE_AIX_MPI],[1],[AIX MPI identifier.])dnl AIX
		;;
	*irix*)
		AC_DEFINE([HAVE_SGI_MPI],[1],[IRIX MPI identifier.])
		dnl No C++ specific MPI calls!
		AX_CXXFLAGS_IRIX_OPTION([-DMPI_NO_CPPBIND])
		;;
	*osf*)
		AC_DEFINE([HAVE_OSF_MPI],[1],[OSF MPI identifier.])dnl OSF
		;;
	*)
		;;
    esac
  fi
  AM_CONDITIONAL([ENABLE_MPI],[test "x$enable_mpi" = xyes])

  dnl Specification maintenance mode; must be enabled to update NIDR
  dnl itself or the specification of programs depending on it
  AC_ARG_ENABLE([spec-maint],
                AS_HELP_STRING([--enable-spec-maint], 
	                       [enable specification maintenance]),
  	        [enable_spec_maint=$enableval], [enable_spec_maint=no])
  AM_CONDITIONAL([ENABLE_SPEC_MAINT], [test "x$enable_spec_maint" = xyes ])

  dnl Usage tracking option check (libcurl is REQUIRED and the macro will
  dnl define LIBCURL and LIBCURL_CPPFLAGS)
  AC_ARG_ENABLE([tracking],
                AS_HELP_STRING([--enable-tracking="URL;proxy"], 
	                       [turn usage tracking on]),
	  	[tracking_url=$enableval], [tracking_url=""])
  if test -n "$tracking_url"; then
    LIBCURL_CHECK_CONFIG([yes],[],[],
                         AC_MSG_ERROR([DAKOTA tracking requires libcurl]))
    AC_DEFINE_UNQUOTED([DAKOTA_USAGE_TRACKING], "$tracking_url",
	               [URL and optional proxy for DAKOTA usage tracking])
  fi
  AM_CONDITIONAL([ENABLE_TRACKING], [test -n "$tracking_url"])

  dnl Matlab option checks for direct interface
  dnl (Matlab is available for Linux, Linux x86_64, Mac OS X, Solaris,
  dnl Windows, and Windows x64 -- eventually try to support all)
  dnl *** requiring user to specify path to Matlab root ***
  AC_ARG_WITH([matlab],
              AS_HELP_STRING([--with-matlab=DIR],
                             [turn Matlab direct interface on (on platforms
	                      supporting Matlab, default off); DIR specifies
			      Matlab installation root]),
              [MATLAB_ROOT=$withval],[MATLAB_ROOT=""])

  if test -n "$MATLAB_ROOT" -a "x$MATLAB_ROOT" != "xno" ; then

    AC_MSG_CHECKING(for Matlab headers and libraries in $MATLAB_ROOT)

    dnl defaults
    HAVE_MATLAB=unknown
    MATLAB_INCL_PATH="$MATLAB_ROOT/extern/include";
    MATLAB_LIBEXT="so";
    MATLAB_LDFLAGS="";

    dnl If building static, Matlab not available
    if test index("$LDFLAGS","-static-libgcc") = "-1"
    then
      dnl OS-dependent variables and checks
      case "${host_os}" in
        *linux*)
	  if test "${host_cpu}" = "x86_64";
	  then
            MATLAB_LIB_PATH="$MATLAB_ROOT/bin/glnxa64";
    	  else
            MATLAB_LIB_PATH="$MATLAB_ROOT/bin/glnx86"; 
	  fi
          ;;
        *darwin*|*macosx*|*apple*)
          MATLAB_LIBEXT="dylib";
    	  MATLAB_LDFLAGS="-Wl,-flat_namespace -undefined suppress"
	  case "${host_cpu}" in
	    i*86|x86_64)
	      MATLAB_LIB_PATH="$MATLAB_ROOT/bin/maci";
	      ;;
	    *)
	      MATLAB_LIB_PATH="$MATLAB_ROOT/bin/mac";
	      ;;
	  esac
	  ;;
        *solaris*)
          MATLAB_LIB_PATH="$MATLAB_ROOT/bin/sol2";
          dnl MATLAB_LIB_PATH="$MATLAB_ROOT/bin/sol64";
 	  ;;
	*cygwin*)
	  case "${host_cpu}" in
	    x86_64)
              MATLAB_LIB_PATH="$MATLAB_ROOT/bin/win64";
	      ;;
	    *)
	      MATLAB_LIB_PATH="$MATLAB_ROOT/bin/win32";
	      ;;
          esac
	  MATLAB_LIBEXT="dll";
	  ;;
        *)
          HAVE_MATLAB=no;
          ;;
      esac
    else
      HAVE_MATLAB=no
    fi
  
    dnl proceed with tests if Matlab still might be available
    if test "x$HAVE_MATLAB" = "xunknown"; 
    then
      dnl The following should work for Linux, Solaris, Mac OS X, and Cygwin.
      dnl (use AC_CHECK_FILE, AC_CHECK_HEADER, and/or AC_CHECK_LIB???)
      if test -f "$MATLAB_INCL_PATH/engine.h" -a \
              -f "$MATLAB_LIB_PATH/libeng.$MATLAB_LIBEXT" -a \
              -f "$MATLAB_LIB_PATH/libmx.$MATLAB_LIBEXT" -a \
              -f "$MATLAB_LIB_PATH/libut.$MATLAB_LIBEXT"      ;
      then
        HAVE_MATLAB=yes
        MATLAB_LIBS="-L$MATLAB_LIB_PATH -leng -lmx -lut"
        MATLAB_INCL="-I$MATLAB_INCL_PATH"
      else
        HAVE_MATLAB=no
      fi
    fi

    AC_MSG_RESULT($HAVE_MATLAB)
    AC_DEFINE([DAKOTA_MATLAB],[1],[with Matlab in DAKOTA src])
    AC_SUBST([MATLAB_INCL])
    AC_SUBST([MATLAB_LIBS])
    AC_SUBST([MATLAB_LDFLAGS])
  else
    HAVE_MATLAB=no
  fi
  AM_CONDITIONAL([WITH_MATLAB], [test "x$HAVE_MATLAB" = xyes])

  dnl ModelCenter option check (for library interface used in Cygwin).
  AC_ARG_WITH([modelcenter],AS_HELP_STRING([--with-modelcenter],
				           [turn MODELCENTER support on]),
              [with_modelcenter=$withval],[with_modelcenter=no])
  if test "x$with_modelcenter" = xyes; then
    if test "x$host_os" = xcygwin -o "x$host_os" = xmingw32; then
      AC_DEFINE([DAKOTA_MODELCENTER],[1],[Macro for ModelCenter code.])
    else
      AC_MSG_ERROR([ModelCenter is only available in Cygwin or MinGW builds.])
    fi
  fi
  AM_CONDITIONAL([WITH_MODELCENTER],
		 [test "x$with_modelcenter" = xyes -a \( "x$host_os" = xcygwin -o "x$host_os" = xmingw32 \) ])

  dnl Plugin option check (for sample plug-in direct application interface).
  AC_ARG_WITH([plugin],AS_HELP_STRING([--with-plugin],
				      [turn PLUGIN support on]),
              [with_plugin=$withval],[with_plugin=no])
  if test "x$with_plugin" = xyes; then
    AC_DEFINE([DAKOTA_PLUGIN],[1],[Macro for Plugin code.])
  fi
  AM_CONDITIONAL([WITH_PLUGIN],[test "x$with_plugin" = xyes])

  dnl DLL option check (for Windows DLL API).
  dnl For now, only supported with cygwin, though can build on other platforms
  AC_ARG_WITH([dll],AS_HELP_STRING([--with-dll],
				   [turn DLL API support on]),
              [with_dll=$withval],[with_dll=no])
  dnl TODO: more generic check for host_os
  if test "x$with_dll" = xyes; then
    AC_DEFINE([DAKOTA_DLL],[1],[Macro for DLL.])
    if test "x$host_os" = xcygwin -o "x$host_os" = xmingw32; then
      DLL_API_XTRALIBS="-shared -Wl,--output-def,dll_api.def,--out-implib,libdll_api.a"
    else
      DLL_API_XTRALIBS=
    fi
    AC_SUBST([DLL_API_XTRALIBS])
  fi
  AM_CONDITIONAL([WITH_DLL],
		 [test "x$with_dll" = xyes])

  dnl Python option check, including test for numpy.
  AC_ARG_WITH([python],AS_HELP_STRING([--with-python],[turn Python support on]),
	      [with_python=$withval],[with_python=no])
  if test "x$with_python" = xyes; then
    dnl contributed macro for Python checks
    AX_PYTHON_DEVEL
    AC_DEFINE([DAKOTA_PYTHON],[1],
	      [Macro to handle code which depends on Python.])
    dnl Check for numpy/arrayobject.h (most likely in platform-specific lib dir)
    python_plat_lib=`$PYTHON -c "import distutils.sysconfig; \
	            print (distutils.sysconfig.get_python_lib(1,0));"`
    save_CPPFLAGS=$CPPFLAGS
    CPPFLAGS="$save_CPPFLAGS $PYTHON_CPPFLAGS -I$python_plat_lib/numpy/core/include/numpy"
    AC_LANG([C++])
    AC_CHECK_HEADERS([arrayobject.h],
      [PYTHON_CPPFLAGS="$PYTHON_CPPFLAGS -I$python_plat_lib/numpy/core/include/numpy"
        AC_SUBST(PYTHON_CPPFLAGS)	
        AC_DEFINE([DAKOTA_PYTHON_NUMPY],[1],
	          [Macro to handle code which depends on Python numpy array.])],
      [AC_MSG_WARN(["arrayobject.h not found for numpy support: using CXXFLAGS=-I/path/to/numpy/includes may be necessary."])],
      [#include <Python.h>])
    CPPFLAGS="$save_CPPFLAGS"
  fi
  AM_CONDITIONAL([WITH_PYTHON], [test "x$with_python" = xyes])

])

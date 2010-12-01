dnl Packages

AC_DEFUN([DAK_PACKAGES],[

  dnl -------------------
  dnl Boost package check
  dnl -------------------
  AX_BOOST_BASE([1.37], [required], [$srcdir/packages/boost], [packages/boost])

  if test "$ac_boost_build_tpl" = "yes"; then
    AC_MSG_NOTICE([will build bundled boost TPL.])
    AC_CONFIG_SUBDIRS([packages/boost])
  else
    AC_MSG_NOTICE([skipping bundled boost TPL.])
  fi

  AC_DEFINE([BOOST_MULTI_INDEX_DISABLE_SERIALIZATION],[1],
            [Macro to disable dependency on the Boost serialization library.])



  dnl -------------------------
  dnl Teuchos include DIR check
  dnl -------------------------
  AC_ARG_WITH([teuchos-include],
              AC_HELP_STRING([--with-teuchos-include=DIR],
                             [use Teuchos headers in specified include DIR]),
              [],[with_teuchos_include="yes"])

  case $with_teuchos_include in

  no)
    AC_MSG_ERROR([Usage: --with-teuchos-include=DIR; without not allowed])
    ;;

  yes | "")
    AC_MSG_CHECKING([for Teuchos include dir via environment variable])
    if test -n "$TEUCHOS_INCLUDE" -a -d "$TEUCHOS_INCLUDE"; then
      dnl could check for existence of header
      AC_MSG_RESULT([using Teuchos headers in TEUCHOS_INCLUDE: $TEUCHOS_INCLUDE])
    else
      AC_MSG_NOTICE([Teuchos include dir not specfied; will find later.])
    fi
    ;;

  *)
    AC_MSG_CHECKING([for specified Teuchos include DIR])
    TEUCHOS_INCLUDE=$withval
    if test -n "$TEUCHOS_INCLUDE" -a -d "$TEUCHOS_INCLUDE"; then
      export TEUCHOS_INCLUDE=$withval
      AC_MSG_RESULT([using Teuchos headers in $TEUCHOS_INCLUDE])
    else
      AC_MSG_ERROR([could not locate specified Teuchos include dir $TEUCHOS_INCLUDE])
    fi
    ;;

  esac

  AC_ARG_VAR(TEUCHOS_INCLUDE,
             [Path to headers for Teuchos, OO Numerics foundation library])


  dnl ---------------------
  dnl Teuchos lib DIR check
  dnl ---------------------
  AC_ARG_WITH([teuchos-lib],
              AC_HELP_STRING([--with-teuchos-lib=DIR],
                             [use Teuchos libraries in specified lib DIR]),
              [],[with_teuchos_lib="yes"])

  case $with_teuchos_lib in

  no)
    AC_MSG_ERROR([Usage: --with-teuchos-lib=DIR; without not allowed])
    ;;

  yes | "")

    AC_MSG_CHECKING([for Teuchos lib dir via environment variable])
    if test -n "$TEUCHOS_LIB" -a -d "$TEUCHOS_LIB"; then
      dnl cannot check for existence of lib in this case,
      dnl since might not be installed
      AC_MSG_RESULT([using Teuchos libs in TEUCHOS_LIB: $TEUCHOS_LIB])
    else
      AC_MSG_NOTICE([Teuchos lib dir not specfied; will find later.])
    fi
    ;;

  *)
    AC_MSG_CHECKING([for specified Teuchos lib DIR])
    TEUCHOS_LIB=$withval
    if test -n "$TEUCHOS_LIB" -a -d "$TEUCHOS_LIB"; then
      export TEUCHOS_LIB=$withval
      AC_MSG_RESULT([using Teuchos libs in $TEUCHOS_LIB])
    else
      AC_MSG_ERROR([could not locate specified Teuchos lib dir $TEUCHOS_LIB])
    fi
    ;;

  esac

  AC_ARG_VAR(TEUCHOS_LIB,
             [Path to libraries for Teuchos OO Numerics foundation library])


  dnl Teuchos package checks: this will perform the final checks to
  dnl resolve all Teuchos-related configure options and make file
  dnl include/lib settings
  AC_ARG_WITH([teuchos],
              AC_HELP_STRING([--with-teuchos=DIR],
                             [use Teuchos (default is yes), optionally 
                              specify the root Teuchos directory containing 
                              src or include/lib]),
              [],[with_teuchos="yes"])

  dnl should also include test for directory either here or above
  acx_valid_inc=0
  if test -n "$TEUCHOS_INCLUDE" -a -d "$TEUCHOS_INCLUDE"; then
    acx_valid_inc=1
  fi
  acx_valid_lib=0
  if test -n "$TEUCHOS_LIB" -a -d "$TEUCHOS_LIB"; then
    acx_valid_lib=1
  fi

  dnl this is cludgy, but not sure how to do booleans nor xor
  dnl is shell arithmetic safe here?
  if test $acx_valid_inc -eq 1 -a $acx_valid_lib -eq 0; then 
    AC_MSG_ERROR([Must specify both or neither of Teuchos include/lib, not exactly one.])
  fi
  if test $acx_valid_inc -eq 0 -a $acx_valid_lib -eq 1; then 
    AC_MSG_ERROR([Must specify both or neither of Teuchos include/lib, not exactly one.])
  fi

  acx_local_teuchos=no
  case $with_teuchos in
  dnl DAKOTA depends on Teuchos UNCONDITIONALLY
  no)
    AC_MSG_ERROR([DAKOTA cannot be configured without Teuchos. Please use a
                 --with-teuchos option OR provide path to a prebuilt Teuchos.])
    ;;

  dnl For yes, check environment variables, otherwise fallback to local Teuchos
  yes | "")
    AC_MSG_CHECKING([for Teuchos])
    if test $acx_valid_inc -eq 1 -a $acx_valid_lib -eq 1; then
      AC_MSG_RESULT([using previously specified Teuchos include/lib])
    elif test -n "$TEUCHOS_ROOT" -a -d "$TEUCHOS_ROOT"; then
      AC_MSG_RESULT([using Teuchos in TEUCHOS_ROOT: $TEUCHOS_ROOT])
    elif test -d `pwd`/packages/teuchos; then
      dnl use local teuchos and instruct subpackages to do so as well
      export TEUCHOS_ROOT=`pwd`/packages/teuchos
      acx_local_teuchos=yes
      AC_CONFIG_SUBDIRS([packages/teuchos])
      AC_MSG_RESULT([using local Teuchos in $TEUCHOS_ROOT])
    else
      AC_MSG_NOTICE([could not find Teuchos directory.])
      AC_MSG_NOTICE([need help locating teuchos!])
      AC_MSG_ERROR([PLEASE PROVIDE full path to teuchos, --with-teuchos=<DIR>])
    fi
    ;;

  dnl Otherwise, user should have provided an explicit path to Teuchos
  *)
    if test $acx_valid_inc -eq 1 -a $acx_valid_lib -eq 1; then
      AC_MSG_ERROR([Teuchos include/lib and root DIR are mutually exclusive])
    fi
    AC_MSG_CHECKING([for specified Teuchos])
    TEUCHOS_ROOT=$withval
    if test -n "$TEUCHOS_ROOT" -a -d "$TEUCHOS_ROOT"; then
      AC_MSG_RESULT([using: $TEUCHOS_ROOT])
    else
      AC_MSG_ERROR([could not locate $TEUCHOS_ROOT])
    fi
    ;;

  esac

  if test $acx_valid_inc -eq 1 -a $acx_valid_lib -eq 1; then

    TEUCHOS_CPPFLAGS="-I$TEUCHOS_INCLUDE -I$TEUCHOS_LIB"
    TEUCHOS_LDFLAGS="-L$TEUCHOS_LIB"

  dnl Check for INSTALLED Teuchos vs. BUILT, but NOT-installed Teuchos
  elif test -n "$TEUCHOS_ROOT" -a -d "$TEUCHOS_ROOT/include" -a -d "$TEUCHOS_ROOT/lib"; then

    AC_MSG_NOTICE([Found an INSTALLED teuchos!])
    TEUCHOS_CPPFLAGS="-I$TEUCHOS_ROOT/include"
    TEUCHOS_LDFLAGS="-L$TEUCHOS_ROOT/lib"

  elif test -n "$TEUCHOS_ROOT" -a -d "$TEUCHOS_ROOT/src"; then

    AC_MSG_NOTICE([Found a source teuchos!])
    TEUCHOS_CPPFLAGS="-I$TEUCHOS_ROOT/src"
    TEUCHOS_LDFLAGS="-L$TEUCHOS_ROOT/src"

  else

    AC_MSG_ERROR([could not find Teuchos library relative to root nor include/lib.])

  fi

  AC_MSG_NOTICE([Final Teuchos config CPPFLAGS: $TEUCHOS_CPPFLAGS LDFLAGS: $TEUCHOS_LDFLAGS])

  AC_ARG_VAR(TEUCHOS_ROOT, [Path to Teuchos, OO Numerics foundation library])
  AC_SUBST(TEUCHOS_CPPFLAGS)
  AC_SUBST(TEUCHOS_LDFLAGS)

  AM_CONDITIONAL([BUILD_TEUCHOS], [test "x$acx_local_teuchos" = xyes])


  dnl GRAPHICS package checks.
  AC_ARG_WITH([graphics],AS_HELP_STRING([--without-graphics],
  					[turn GRAPHICS support off]),
	      [with_graphics=$withval],[with_graphics=yes])
  if test "x$with_graphics" = xyes; then
    dnl on most platforms, when linking dynamically, it suffices to link
    dnl against pthreads, Xpm, Xt, and Xm (the direct dependencies of motif),
    dnl however exceptions for static linking or specific platforms are
    dnl indicated below
    AC_MSG_NOTICE([DAKOTA graphics: begin checking for dependencies.])
    dakota_graphics_ok=yes
    dnl pthread is required by plplot
    ACX_PTHREAD(,dakota_graphics_ok=no)
    dnl AC_PATH_XTRA is AC_REQUIREd by AC_FIND_XPM so its vars are available,
    dnl e.g, have_x.  TODO: rigorous check for Xpm success
    AC_FIND_XPM 
    dnl X11 mandatory, though not necessary to specify when dynamically linking
    AC_CHECK_LIB([X11],[XDrawLines],
		 [X_EXTRA_LIBS="-lX11 $X_EXTRA_LIBS"],
		 dakota_graphics_ok=no,
		 [$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS])
    case "${host_os}" in
	*cygwin*|*darwin*|*linux*)
            dnl these may be required to satisfy dependencies in the static
	    dnl link case (otherwise don't need to be on the link line)
	    AC_CHECK_LIB([Xext],[XShapeCombineMask],
			 [X_EXTRA_LIBS="-lXext $X_EXTRA_LIBS"],
			 dakota_graphics_ok=no,
			 [$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS])
	    AC_CHECK_LIB([Xp],[XpGetContext],
			 [X_EXTRA_LIBS="-lXp $X_EXTRA_LIBS"],
			 dakota_graphics_ok=no,
			 [$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS])
	;;
	*osf*|*solaris*)
	    dnl these two may be required for dependencies, but don't require
            dnl calling out on the link line -- make them optional
	    AC_CHECK_LIB([Xau],[XauFileName],
			 [X_EXTRA_LIBS="-lXau $X_EXTRA_LIBS"],,
			 [$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS])
	    AC_CHECK_LIB([Xaw],[XawInitializeWidgetSet],
			 [X_EXTRA_LIBS="-lXaw $X_EXTRA_LIBS"],,
			 [$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS])
	;;
	*)
	;;
    esac
    dnl libXmu required to satisfy dependencies (in static link case) and
    dnl        required to specify explicitly on solaris
    AC_CHECK_LIB([Xmu],[XmuClientWindow],
	         [X_EXTRA_LIBS="-lXmu $X_EXTRA_LIBS"],
		 dakota_graphics_ok=no,
		 [$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS])
    dnl libXt and libXm are required directly by motif
    AC_CHECK_LIB([Xt],[XtAppInitialize],
		 [X_EXTRA_LIBS="-lXt $X_EXTRA_LIBS"],
		 dakota_graphics_ok=no,
		 [$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS])
    AC_CHECK_LIB([Xm],[XmCreateOptionMenu],
		 [X_EXTRA_LIBS="-lXm $X_EXTRA_LIBS"],
		 dakota_graphics_ok=no,
		 [$X_LIBS $X_EXTRA_LIBS $X_PRE_LIBS])
    dnl only proceed if we passed all checks
    if test x"$dakota_graphics_ok" = xno -o x"$have_x" != xyes; then
      AC_MSG_ERROR([Could not find one or more dependencies needed for DAKOTA graphics; make sure necessary headers and libraries are available or consider configuring --without-graphics.])
    fi
    AC_MSG_NOTICE([DAKOTA graphics: enabled.])
    AC_CONFIG_SUBDIRS([packages/motif])
    AC_DEFINE([DAKOTA_GRAPHICS],[1],
	      [Macro to handle code which depends on motif.])

  fi
  AM_CONDITIONAL([WITH_GRAPHICS],[test "x$with_graphics" = xyes])

  dnl NIDR is unconditional
  AC_CONFIG_SUBDIRS([packages/nidr])

  dnl Pecos package check.
  dnl AC_ARG_WITH([pecos],AS_HELP_STRING([--without-pecos],
  dnl 	      [turn Pecos support off]),[with_pecos=$withval],[with_pecos=yes])
  dnl if test "x$with_pecos" = xyes; then
     AC_CONFIG_SUBDIRS([packages/pecos])
     AC_DEFINE([DAKOTA_PECOS],[1],
	       [Macro to handle code which depends on PECOS.])
  dnl fi
  dnl AM_CONDITIONAL([WITH_PECOS],[test "x$with_pecos" = xyes])

  dnl AMPL package check.
  AC_ARG_WITH([ampl],AS_HELP_STRING([--without-ampl],[omit AMPL/solver interface library]),
	      [with_ampl=$withval],[with_ampl=yes])
  if test "x$with_ampl" = xyes; then
    AC_MSG_NOTICE([DAKOTA configured with AMPL.])
    AC_CONFIG_SUBDIRS([packages/ampl])
    AC_DEFINE([HAVE_AMPL],[1],[Macro to handle code which depends on AMPL.])
  else
    AC_MSG_NOTICE([DAKOTA configured without AMPL.])
  fi
  AM_CONDITIONAL([WITH_AMPL],[test "x$with_ampl" = xyes])

  dnl SURFPACK package check.
  AC_ARG_WITH([surfpack],AS_HELP_STRING([--without-surfpack],
					[turn SURFPACK support off]),
	      [with_surfpack=$withval],[with_surfpack=yes])
  if test "x$with_surfpack" = xyes -a -d $srcdir/packages/surfpack; then
    AC_CONFIG_SUBDIRS([packages/surfpack])
    AC_DEFINE([DAKOTA_SURFPACK],[1],
	      [Macro to handle code which depends on SURFPACK.])
    MAYBE_SURFPACK=surfpack
  else
    MAYBE_SURFPACK=
  fi
  AC_SUBST([MAYBE_SURFPACK])
  AM_CONDITIONAL([WITH_SURFPACK],[test "x$with_surfpack" = xyes -a \
				       -d $srcdir/packages/surfpack])

  dnl GSL package check.
  AC_ARG_WITH([gsl],
              AC_HELP_STRING([--with-gsl<=DIR>],
                             [use GPL package GSL (default no); optionally 
                              specify the root DIR for GSL include/lib]),
              [],[with_gsl="no"])

  acx_gsl_ok=disable
  case $with_gsl in
    no) ;;
    yes | "") acx_gsl_ok=want ;;
    *)
      if test -d "$withval" -a -d "$withval/include" -a -d "$withval/lib"; then
	acx_gsl_ok=want
        GSL_CPPFLAGS="-I$withval/include"
        GSL_LDFLAGS="-L$withval/lib"
      else
        AC_MSG_ERROR([specified GSL directory $withval must exist and contain 
                      include/ and lib/])
      fi
      ;;
  esac

  dnl prepend user-specified location then search for GSL
  dnl no granularity to notify user which used
  if test "x$acx_gsl_ok" = xwant; then

    AC_MSG_NOTICE([checking for GSL...])

    save_CPPFLAGS="$CPPFLAGS"
    save_LDFLAGS="$LDFLAGS"
    if test -n $GSL_CPPFLAGS; then
      CPPFLAGS="$CPPFLAGS $GSL_CPPFLAGS"
      LDFLAGS="$LDFLAGS $GSL_LDFLAGS"
    fi

    dnl use GSL_LIBS, so we don't pollute $LIBS with AC_CHECK_LIB
    acx_gsl_ok=yes;
    GSL_LIBS=
    AC_CHECK_LIB([m],[cos], [GSL_LIBS="-lm $GSL_LIBS"], acx_gsl_ok=no)
    AC_CHECK_LIB([gslcblas],[cblas_dgemm], [GSL_LIBS="-lgslcblas $GSL_LIBS"], acx_gsl_ok=no, [$GSL_LIBS])
    AC_CHECK_LIB([gsl],[gsl_ran_fdist_pdf], [GSL_LIBS="-lgsl $GSL_LIBS"], acx_gsl_ok=no, [$GSL_LIBS])
    AC_CHECK_HEADERS([gsl/gsl_randist.h],,acx_gsl_ok=no)

    if test "x$acx_gsl_ok" = xyes; then
      AC_MSG_NOTICE([GNU GPL package GSL found])
      AC_MSG_NOTICE([NOTE: your build includes GNU GPL (binary) library GSL!])
      dnl AC_DEFINE([HAVE_GSL],[1],[Macro to handle code which depends on GSL.])
      AC_SUBST(GSL_CPPFLAGS)
      AC_SUBST(GSL_LDFLAGS)
      AC_SUBST(GSL_LIBS)
    else
      AC_MSG_ERROR([GSL requested but not found])
    fi

    CPPFLAGS="$save_CPPFLAGS"
    LDFLAGS="$save_LDFLAGS"

  fi
  AM_CONDITIONAL([WITH_GSL], [test "x$acx_gsl_ok" = xyes])

])

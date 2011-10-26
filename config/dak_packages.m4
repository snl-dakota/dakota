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

  AC_DEFINE([DAKOTA_HAVE_BOOST_FS],[1],
            [Macro to enable dependency on the Boost filesystem library.])

  dnl ---------------------
  dnl Teuchos package check
  dnl ---------------------
  DAKOTA_AC_TEUCHOS()


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
    AC_DEFINE([HAVE_X_GRAPHICS],[1],
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
     AC_DEFINE([HAVE_PECOS],[1],
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
    AC_DEFINE([HAVE_SURFPACK],[1],
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

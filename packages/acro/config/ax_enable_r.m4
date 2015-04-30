AC_DEFUN([AX_ENABLE_R],
[
  # default R to off; don't throw error if not found
  AC_ARG_ENABLE(r,
    AS_HELP_STRING([--enable-r],[Enable R statistics package]),
    [ax_enable_r=$enableval],
    [ax_enable_r=no]
  )
 
  ax_enable_r_found="no"
  if test "X$ax_enable_r" = "Xyes" ; then
    r_version=`R --version`
    if test "$r_version" != ""; then
      ax_enable_r_found="yes"
      R_INCLUDE=`R CMD config --cppflags`
      R_LIBS=`R CMD config --ldflags`
      AC_DEFINE(HAVE_R,,[define if the R statistics package is available])
      AC_MSG_NOTICE([R statistics package found.])
    else
      AC_MSG_NOTICE([skipping R statistics package.])
    fi
  fi

  AC_ARG_VAR(R_INCLUDE, [Path to R development header files])
  AC_ARG_VAR(R_LIBS,  [Path to R link libraries])

  AM_CONDITIONAL([HAVE_R], test "$ax_enable_r_found" = "yes")

])

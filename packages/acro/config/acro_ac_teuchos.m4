dnl @synopsis ACRO_AC_TEUCHOS()
dnl  
dnl Defines
dnl   arg var  TEUCHOS_ROOT
dnl   ac subst TEUCHOS_CPPFLAGS, TEUCHOS_LDFLAGS
dnl   am cond  BUILD_TEUCHOS
AC_DEFUN([ACRO_AC_TEUCHOS],
[

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
  dnl Package depends on Teuchos UNCONDITIONALLY
  no)
    AC_MSG_ERROR([${PACKAGE_NAME} cannot be configured without Teuchos. Please use a
                 --with-teuchos option OR provide path to a prebuilt Teuchos.])
    ;;

  dnl For yes, check environment variables, otherwise fallback to local Teuchos
  yes | "")
    AC_MSG_CHECKING([for Teuchos])
    if test $acx_valid_inc -eq 1 -a $acx_valid_lib -eq 1; then
      AC_MSG_RESULT([using previously specified Teuchos include/lib])
    elif test -n "$TEUCHOS_ROOT" -a -d "$TEUCHOS_ROOT"; then
      AC_MSG_RESULT([using Teuchos in TEUCHOS_ROOT: $TEUCHOS_ROOT])
    elif test -d `pwd`/tpl/teuchos; then
      dnl use local teuchos and instruct subpackages to do so as well
      export TEUCHOS_ROOT=`pwd`/tpl/teuchos
      acx_local_teuchos=yes
      AC_CONFIG_SUBDIRS([tpl/teuchos])
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
  if test "x$acx_local_teuchos" = xyes; then
    TEUCHOS_DIR=${TEUCHOS_ROOT}
  fi
  AC_SUBST(TEUCHOS_DIR)

  AM_CONDITIONAL([BUILD_TEUCHOS], [test "x$acx_local_teuchos" = xyes])

])


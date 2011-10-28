# If Boost version is strictly less than version specified in $1
# set dakota_ac_boost_lt_version_specified=yes
# else set dakota_ac_boost_lt_version_specified=no
AC_DEFUN([DAKOTA_AC_BOOST_LT_VERSION],
[
  boost_lib_version_max="$1"
  boost_lib_version_max_shorten=`expr $boost_lib_version_max : '\([[0-9]]*\.[[0-9]]*\)'`
  boost_lib_version_max_major=`expr $boost_lib_version_max : '\([[0-9]]*\)'`
  boost_lib_version_max_minor=`expr $boost_lib_version_max : '[[0-9]]*\.\([[0-9]]*\)'`
  boost_lib_version_max_sub_minor=`expr $boost_lib_version_max : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
  if test "x$boost_lib_version_max_sub_minor" = "x" ; then
  	boost_lib_version_max_sub_minor="0"
  fi
  WANT_BOOST_VERSION=`expr $boost_lib_version_max_major \* 100000 \+  $boost_lib_version_max_minor \* 100 \+ $boost_lib_version_max_sub_minor`

  CPPFLAGS_SAVED="$CPPFLAGS"
  LDFLAGS_SAVED="$LDFLAGS"
  
  AC_MSG_CHECKING([whether boost version < $boost_lib_version_max])
  
  dnl these must already prefixed with -I -L from ax_boost_base
  if test "$BOOST_CPPFLAGS" != ""; then
  	CPPFLAGS="$CPPFLAGS_SAVED $BOOST_CPPFLAGS"
  fi
  if test "$BOOST_LDFLAGS" != ""; then
  	LDFLAGS="$LDFLAGS_SAVED -L$BOOST_LDFLAGS"
  fi
  
  AC_LANG_PUSH(C++)
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
  @%:@include <boost/version.hpp>
  ]], [[
  #if BOOST_VERSION < $WANT_BOOST_VERSION
  // Everything is okay
  #else
  #  error Boost version is newer
  #endif
  ]])],[
  AC_MSG_RESULT([yes])
  dakota_ac_boost_lt_version_specified=yes
  ],[
  AC_MSG_RESULT([no])
  dakota_ac_boost_lt_version_specified=no
  ])
  AC_LANG_POP([C++])
  
  CPPFLAGS="$CPPFLAGS_SAVED"
  LDFLAGS="$LDFLAGS_SAVED"
])

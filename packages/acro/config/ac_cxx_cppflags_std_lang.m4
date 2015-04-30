dnl @synopsis AC_CXX_CPPFLAGS_STD_LANG(CPP-FLAGS)
dnl
dnl Append to CPP-FLAGS the set of flags that should be passed to the
dnl C++ preprocessor in order to enable use of C++ features as defined
dnl in the ANSI C++ standard (eg. use of standard iostream classes in
dnl the `std' namespace, etc.).
dnl
dnl @category Cxx
dnl @author Ludovic Courtès <ludo@chbouib.org>
dnl @version 2004-09-07
dnl @license AllPermissive

AC_DEFUN([AC_CXX_CPPFLAGS_STD_LANG],
  [AC_REQUIRE([AC_CXX_COMPILER_VENDOR])
   case "$ac_cv_cxx_compiler_vendor" in
     compaq)
      # By default, Compaq CXX has an iostream classes implementation
      # that is _not_ in the `std' namespace.
      $1="$$1 -D__USE_STD_IOSTREAM=1";;
     hp)
      # When `configure' looks for a C++ header (eg. <iostream>) it invokes
      # `aCC -E $CPPFLAGS'.  However, including standard C++ headers like
      # <iostream> fails if `-AA' is not supplied.
      $1="$$1 -AA";;
   esac])

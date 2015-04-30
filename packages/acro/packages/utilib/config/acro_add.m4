dnl Add flags to a list taking care to avoid spaces at beginning
dnl and end of variable.  This is necessary to support use of
dnl cache variables.  User may run configure again, with flags
dnl listed on command line, but if they had extra spaces when computed
dnl by configure in the cache file, configure will exit with an error
dnl because they don't match.
dnl
dnl  cppflags=-g
dnl  newflag=-x
dnl
dnl  ACRO_ADD(cppflags, $newflag)
dnl
dnl  sets cppflags to "-x -g"
dnl
dnl If "newflag" is either empty or equal to "notset", we do nothing.
dnl

AC_DEFUN([ACRO_ADD],[

if test -z "$2" || test "X$2" = "Xnotset" ; then : ; else
  using_acro_special=yes
  if test -z "$$1" ; then 
    $1="$2"
  else
    $1="$2 $$1"
  fi
fi
])

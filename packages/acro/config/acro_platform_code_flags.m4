dnl ACRO platform flags tested in source code

AC_DEFUN([ACRO_PLATFORM_CODE_FLAGS],[
  dnl
  dnl Determine special platform flags that are tested in our sources.
  dnl Do not include compiler, preprocessor, etc. flags.  Those
  dnl belong in acro_platform_compiler_flags.m4.
  dnl TODO: Create tests for NO_MEMBER_TEMPLATE_FRIENDS

echo "OS $host_os"

case $host_os in

  # DEC OSF
  *osf*)

    case $CXX in
      cxx* ) AC_DEFINE(OSF_CC,,[special platform flag]) ;;
    esac
  ;;

  # IBM test
  *aix*)

    case $CXX in
      xlC* ) AC_DEFINE(AIX_CC,,[special platform flag]) ;;
    esac
  ;;

  # SGI/IRIX test
  irix* )

    if test "${CXX}" = "CC"; then
      AC_DEFINE(SGI_CC,,[special platform flag])
    fi

  ;;

  solaris* )

    if test "${CXX}" = "CC"; then
       AC_DEFINE(SOLARIS_CC,,[special platform flag])
       AC_DEFINE(NO_MEMBER_TEMPLATE_FRIENDS,,[special platform flag])
    fi

  ;;

esac

])

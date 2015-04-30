dnl ACRO platform flags for compiler, preprocessor, etc.

AC_DEFUN([ACRO_PLATFORM_COMPILER_FLAGS],[
  dnl
  dnl Determine special platform preprocessor and compiler flags.
  dnl Do not include platform specific flags that are tested in our
  dnl code, but not interpreted by compiler, etc.  Those flags belong
  dnl in acro_platform_code_flags.m4.

echo "OS $host_os"

case $host_os in

  # IBM test
  *aix*)

    case $CXX in
      xlC* ) SPECIAL_CXXFLAGS="-qnolm -qrtti";;
    esac
  ;;

  # SGI/IRIX test
  irix* )

    if test "${CXX}" = "CC"; then
     SPECIAL_CFLAGS="-LANG:std -LANG:ansi-for-init-scope=ON"
     SPECIAL_CXXFLAGS="-LANG:std -LANG:ansi-for-init-scope=ON"
     F77_MISC=f77
     FFLAGS_MISC="-64"
     SPECIAL_AR="CC -ar -W,l -W,s -o"
    fi

  ;;

  solaris* )

    if test "${CXX}" = "CC"; then
       SPECIAL_AR="CC -xar -o";
       WITH_OPT=`echo "${WITH_OPT}" | sed 's/-O\([[0-9]]*\)/-xO\1/g'`;
       optimization_default=`echo "${optimization_default}" | sed 's/-O\([[0-9]]*\)/-xO\1/g'`;
    fi

  ;;

esac

])

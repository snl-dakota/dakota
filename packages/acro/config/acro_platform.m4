dnl ACRO platform macro

AC_DEFUN([ACRO_PLATFORM],[
  dnl
  dnl Use config.guess to define
  dnl
  dnl
  dnl   $build_cpu, $build_vendor, $build_os
  dnl   $host_cpu, $host_vendor, $host_os
  dnl   $target_cpu, $target_vendor, $target_os
  dnl   (example: i686, pc, linux-gnu or sparc, sun, solaris2.8)
  dnl

AM_CONDITIONAL(BUILD_APPLE, false)
AM_CONDITIONAL(HOST_APPLE, false)
AM_CONDITIONAL(TARGET_APPLE, false)

AM_CONDITIONAL(BUILD_SOLARIS, false)
AM_CONDITIONAL(HOST_SOLARIS, false)
AM_CONDITIONAL(TARGET_SOLARIS, false)

AM_CONDITIONAL(BUILD_MINGW, false)
AM_CONDITIONAL(HOST_MINGW, false)
AM_CONDITIONAL(TARGET_MINGW, false)

AM_CONDITIONAL(BUILD_LINUX, false)
AM_CONDITIONAL(HOST_LINUX, false)
AM_CONDITIONAL(TARGET_LINUX, false)

AM_CONDITIONAL(BUILD_CYGWIN, false)
AM_CONDITIONAL(HOST_CYGWIN, false)
AM_CONDITIONAL(TARGET_CYGWIN, false)

AM_CONDITIONAL(BUILD_CPU_X86, false)
AM_CONDITIONAL(BUILD_CPU_X86_64, false)
AM_CONDITIONAL(BUILD_CPU_SPARC, false)

case $build_cpu in
  i*86_64)
    AM_CONDITIONAL(BUILD_CPU_X86_64, true)
    AC_DEFINE(BUILD_CPU_X86_64,1,[software build cpu is 64 bit x86])
    ;;
  i*86)
    AM_CONDITIONAL(BUILD_CPU_X86, true)
    AC_DEFINE(BUILD_CPU_X86,1,[software build cpu is x86])
    ;;
  sparc)
    AM_CONDITIONAL(BUILD_CPU_SPARC, true)
    AC_DEFINE(BUILD_CPU_SPARC,1,[software build cpu is sparc])
    ;;
esac

case $build_os in
  cygwin)
    AM_CONDITIONAL(BUILD_CYGWIN, true)
    AC_DEFINE(BUILD_CYGWIN,1,[software build os is cygwin])
    ;;
  linux*)
    AM_CONDITIONAL(BUILD_LINUX, true)
    AC_DEFINE(BUILD_LINUX,1,[software build os is linux])
    ;;
  *mingw32*)
    AM_CONDITIONAL(BUILD_MINGW, true)
    AC_DEFINE(BUILD_MINGW,1,[software build os is mingw])
    ;;
  solaris*)
    AM_CONDITIONAL(BUILD_SOLARIS, true)
    AC_DEFINE(BUILD_SOLARIS,1,[software build os is solaris])
    ;;
esac

case $host_os in
  *gnu)
    AM_CONDITIONAL(HOST_GNU, true)
    AC_DEFINE(HOST_GNU,1,[software host is GNU])
    ;;
  *)
    AM_CONDITIONAL(HOST_GNU, false)
    ;;
esac

case $host_os in
  cygwin)
    AM_CONDITIONAL(HOST_CYGWIN, true)
    AM_CONDITIONAL(TARGET_CYGWIN, true)
    AC_DEFINE(HOST_CYGWIN,1,[software host will be cygwin])
    AC_DEFINE(TARGET_CYGWIN,1,[software target will be cygwin])
    ;;
  linux*)
    AM_CONDITIONAL(HOST_LINUX, true)
    AM_CONDITIONAL(TARGET_LINUX, true)
    AC_DEFINE(HOST_LINUX,1,[software host will be linux])
    AC_DEFINE(TARGET_LINUX,1,[software target will be linux])
    ;;
  *mingw*)
    AM_CONDITIONAL(HOST_MINGW, true)
    AM_CONDITIONAL(TARGET_MINGW, true)
    AC_DEFINE(HOST_MINGW,1,[software host will be mingw])
    AC_DEFINE(TARGET_MINGW,1,[software target will be mingw])
    ;;
  solaris*)
    AM_CONDITIONAL(HOST_SOLARIS, true)
    AM_CONDITIONAL(TARGET_SOLARIS, true)
    AC_DEFINE(HOST_SOLARIS,1,[software host will be solaris])
    AC_DEFINE(TARGET_SOLARIS,1,[software target will be solaris])
    ;;
esac

])

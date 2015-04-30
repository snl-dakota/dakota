#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

# Prerequisites.
AC_PREREQ([2.59])
AC_INIT([AMPL],[20080414],[dmgay@sandia.gov])
AC_CONFIG_SRCDIR([asl.h])
AM_INIT_AUTOMAKE([1.9 foreign])
AC_LANG([C])
# Future feature (for now, we rely on ACLOCAL_AMFLAGS in Makefile.am):
#AC_CONFIG_MACRO_DIR([../../m4])
#AC_CONFIG_MACRO_DIR([libltdl/m4])

# Since we're using libtool, don't build shared libraries!
AC_DISABLE_SHARED

# Checks for programs.
AM_PROG_AS
AC_PROG_CC
AC_PROG_INSTALL

# Tells autoconf that libtool will be used.
AC_LANG_PUSH([C])
AC_PROG_LIBTOOL
AC_LANG_POP([C])

xx_Darwin=false
xx_SunOS_i386=false
xx_SunOS_i86pc=false
case `uname -s` in
	Darwin)	xx_Darwin=true;;
	SunOS) case `uname -i` in
		i386)  xx_SunOS_i386=true;;
		i86pc) xx_SunOS_i86pc=true;;
		esac;;
	esac
AM_CONDITIONAL([WITH_Darwin],$xx_Darwin)
AM_CONDITIONAL([WITH_SunOS_i386],$xx_SunOS_i386)
AM_CONDITIONAL([WITH_SunOS_i86pc],$xx_SunOS_i86pc)

# Write the Makefiles and exit.
AC_CONFIG_FILES([Makefile2:Makefile.in])
AC_OUTPUT

# finish up
CC=$CC
CFLAGS=$CFLAGS
if [[ "$srcdir" = "." ]] || [[ "$srcdir" = `pwd` ]]; then
   ./configurehere
else
   $srcdir/configure0
fi
# makefile and Makefile are the same under MS Windows and MacOSX
mv makefile makefile1
sed 's/^clean:/Clean:/;/^\.c\.o/d;/\*\.c/d' makefile1 >Makefile
# bypass a bug in Gnu make by adding an explicit rule...
cat <<\! >>Makefile

funcadd0.o: funcadd0.c
	$(CC) -c $(CFLAGS) funcadd0.c

!
sed 's/^clean:/& Clean /;s/^all:/ALL:/;s@^CFLAGS =.*\$@#&@' Makefile2 >>Makefile
cat >>Makefile <<\!

install:
	echo 'This directory is meant to be used where it is.  See the README* files.'
!
rm makefile1 Makefile2

# work around a pgcc optimization bug

case $CC in pgcc)
	sed 's/^\(CFLAGS =.*\)-O2/\1/' Makefile >Makefile2
	mv Makefile2 Makefile
	;; esac

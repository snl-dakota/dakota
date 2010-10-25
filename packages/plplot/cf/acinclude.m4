dnl $Id: acinclude.m4 3186 2006-02-15 18:17:33Z slbrow $ --*-Autoconf-*--
dnl
dnl Local autoconf extensions.  These are based on the autoconf builtin
dnl macros, and you can do what you want with them.
dnl
dnl Maurice LeBrun
dnl IFS, University of Texas at Austin
dnl 14-Jul-1994
dnl
dnl Copyright (C) 2003, 2004  Rafael Laboissiere
dnl Copyright (C) 2004  Alan W. Irwin
dnl
dnl This file is part of PLplot.
dnl
dnl PLplot is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU General Library Public License as published
dnl by the Free Software Foundation; either version 2 of the License, or
dnl (at your option) any later version.
dnl
dnl PLplot is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU Library General Public License for more details.
dnl
dnl You should have received a copy of the GNU Library General Public License
dnl along with PLplot; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
dnl
dnl ------------------------------------------------------------------------
dnl Include macro which knows about the cf/ directory
dnl
define([PL_INCLUDE], [builtin([include], [cf/$1])])
dnl
dnl ------------------------------------------------------------------------
dnl This quicky is good during development, e.g. PL_IGNORE([ ... ]) to
dnl ignore everything inside the brackets.
dnl
AC_DEFUN([PL_IGNORE],)
dnl
dnl ------------------------------------------------------------------------
dnl PL_ARG_ENABLE(enable-option, help-string, default-value)
dnl
AC_DEFUN([PL_ARG_ENABLE],[
  AC_ARG_ENABLE($1,
    AS_HELP_STRING([--enable-$1], [$2]m4_if($3,,[],[ @<:@default=$3@:>@])),
    [enable_[]translit($1, [-], [_])=$enableval],
    [enable_[]translit($1, [-], [_])=$3])])
dnl ------------------------------------------------------------------------
dnl PL_ARG_WITH(enable-option, help-string, default-value)
dnl
AC_DEFUN([PL_ARG_WITH],[
  AC_ARG_WITH($1,
    AS_HELP_STRING([--with-$1], [$2]m4_if($3,,[],[ @<:@default=$3@:>@])),
    [with_[]translit($1, [-], [_])=$withval],
    [with_[]translit($1, [-], [_])=$3])])
dnl ------------------------------------------------------------------------
dnl Add device/driver information to help string and *_DRIVERS list
dnl
dnl PL_ADD_DRIVER(device, driver, enable_default)
dnl
define([PL_ARG_ENABLE_DRIVER],
  [PL_ARG_ENABLE($1, [enable $1 device driver], $3)])
dnl
dnl ------------------------------------------------------------------------
dnl Recursive macro to set up all driver help entries
dnl
dnl PL_DRIVERS(device:driver:enable_default, ...)
dnl
define(PL_ARG_ENABLE_DRIVERS,[ifelse($1,,,dnl
[PL_ARG_ENABLE_DRIVER(patsubst($1, [:.*], []),dnl
patsubst(patsubst($1, [^[^:]+:], []), [:.*], []),dnl
patsubst($1, [^.*:], []))dnl
PL_ARG_ENABLE_DRIVERS(builtin([shift],$*))])])
dnl
dnl ------------------------------------------------------------------------
define([PL_ADD_DRIVER], [
PL_ARG_ENABLE($1, [enable $1 device driver], $3)
if test "$enable_$1" = yes -o "$enable_$1 $enable_drivers" = " yes" ; then

    enable_$1="yes"

    if test "$enable_dyndrivers" = yes ; then
        pl_suffix=la
	pl_drivers_list="$DYNAMIC_DRIVERS"
    else
        pl_suffix=lo
	pl_drivers_list="$STATIC_DRIVERS"
    fi

    found=0
    for pl_drv in $pl_drivers_list ; do
        if test "$pl_drv" = "$2.$pl_suffix" ; then
	    found=1
	    break
	fi
    done

    if test $found = 0; then
        if test -z "$pl_drivers_list" ; then
            pl_drivers_list="$2.$pl_suffix"
	else
	    pl_drivers_list="$pl_drivers_list $2.$pl_suffix"
	fi
        echo -n $pl_driver
    fi

    if test "$enable_dyndrivers" = yes ; then
	DYNAMIC_DRIVERS="$pl_drivers_list"
    else
	STATIC_DRIVERS="$pl_drivers_list"
    fi

    AC_DEFINE(PLD_$1, [], [Define if $1 driver is present])
    if test -z "$pl_drivers_list" ; then
        DEVICES="$1"
    else
        DEVICES="$DEVICES $1"
    fi

else

    enable_$1="no"

fi
])
dnl
dnl ------------------------------------------------------------------------
dnl Recursive macro to set up all driver help entries
dnl
dnl PL_DRIVERS(device:driver:enable_default, ...)
dnl
define(PL_ADD_DRIVERS,[ifelse($1,,,dnl
[PL_ADD_DRIVER(patsubst($1, [:.*], []),dnl
patsubst(patsubst($1, [^[^:]+:], []), [:.*], []),dnl
patsubst($1, [^.*:], []))dnl
PL_ADD_DRIVERS(builtin([shift],$*))])])
dnl
dnl ------------------------------------------------------------------------
dnl The following macros search a list of directories for the given
dnl include file and takes appropriate actions if found or not.
dnl Arguments:
dnl 	$1 - the include file name, the part before the .h
dnl	$2 - a variable that holds the matched directory name
dnl	$3 - a variable indicating if the search succeeded ("yes"/"no")
dnl	     (if missing, we exit)
dnl Use just PL_FIND_INC, or the PL_FIND_INC_<...> set for more control.
dnl
define(PL_FIND_INC_BEGIN, [
    AC_MSG_CHECKING(for $1.h)
    $2=""
])
define(PL_FIND_INC_SET, [
    for dir in $incdirs; do
	if test -r "$dir/$1.h"; then
	    $2="$dir"
	    AC_MSG_RESULT($dir/$1.h)
	    break
	fi
    done
])
define(PL_FIND_INC_END, [
    if test -z "$$2"; then
	ifelse($3,,[
	    AC_MSG_RESULT(not found -- exiting)
	    exit 1
	],[
	    AC_MSG_RESULT(no)
	    AC_MSG_RESULT([warning: can't find $1.h, setting $3 to no])
	    $3="no"
	])
    fi
    if test "$$2" = "/usr/include"; then
	$2="default"
    fi
])
define(PL_FIND_INC, [
    PL_FIND_INC_BEGIN($*)
    PL_FIND_INC_SET($*)
    PL_FIND_INC_END($*)
])
dnl ------------------------------------------------------------------------
dnl The following macro searches a list of directories for the given
dnl library file and takes appropriate actions if found or not.
dnl Use just PL_FIND_LIB, or the PL_FIND_LIB_<...> set for more control.
dnl
dnl Arguments:
dnl 	$1 - the library name, the part after the -l and before the "."
dnl	$2 - a variable that holds the matched directory name
dnl
dnl PL_FIND_LIB_SET takes:
dnl	$3 - a variable that holds the matched library name in a form
dnl	     suitable for input to the linker (without the suffix, so that
dnl	     any shared library form is given preference).
dnl
dnl PL_FIND_LIB_END takes:
dnl	$3 - a variable indicating if the search succeeded ("yes"/"no")
dnl	     (if missing, we exit)
dnl
dnl PL_FIND_LIB takes these as $3 and $4, respectively.
dnl
define(PL_FIND_LIB_BEGIN, [
    AC_MSG_CHECKING(for lib$1)
    $2=""
])
define(PL_FIND_LIB_SET, [
    for dir in $libdirs; do
	if test -z "$LIBEXTNS"; then
	    LIBEXTNS="so a"
	fi
	for suffix in $LIBEXTNS; do
	    if test -f "$dir/lib$1.$suffix"; then
		$2="$dir"
		$3="-l$1"
		AC_MSG_RESULT($dir/lib$1.$suffix)
		break 2
	    fi
	done
    done
])
define(PL_FIND_LIB_END, [
    if test -z "$$2"; then
	ifelse($3,,[
	    AC_MSG_ERROR(not found -- exiting)
	],[
	    AC_MSG_RESULT(no)
	    AC_MSG_WARN([can't find lib$1, setting $3 to no])
	    $3="no"
	])
    fi
    if test "$$2" = "/usr/lib"; then
	$2="default"
    fi
])
define(PL_FIND_LIB, [
    PL_FIND_LIB_BEGIN($1, $2)
    PL_FIND_LIB_SET($1, $2, $3)
    PL_FIND_LIB_END($1, $2, $4)
])
dnl ------------------------------------------------------------------------
dnl The following macro makes it easier to add includes without adding
dnl redundant -I specifications (to keep the build line concise).
dnl Arguments:
dnl 	$1 - the searched directory name
dnl	$2 - a variable that holds the include specification
dnl	$3 - a variable that holds all the directories searched so far
dnl
define([PL_ADD_TO_INCS],[
    INCSW=""
    if test "$1" != "default"; then
	INCSW="-I$1"
    fi
    for dir in $$3; do
	if test "$1" = "$dir"; then
	    INCSW=""
	    break
	fi
    done
    if test -n "$INCSW"; then
	$2="$$2 $INCSW"
    fi
    $3="$$3 $1"
])
dnl ------------------------------------------------------------------------
dnl The following macro makes it easier to add libs without adding
dnl redundant -L and -l specifications (to keep the build line concise).
dnl Arguments:
dnl 	$1 - the searched directory name
dnl	$2 - the command line option to give to the linker (e.g. -lfoo)
dnl	$3 - a variable that holds the library specification
dnl	$4 - a variable that holds all the directories searched so far
dnl
define([PL_ADD_TO_LIBS],[
    LIBSW=""
    if test "$1" != "default"; then
	LIBSW="-L$1"
    fi
    for dir in $$4; do
	if test "$1" = "$dir"; then
	    LIBSW=""
	    break
	fi
    done
    LIBL="$2"
    for lib in $$3; do
	if test "$2" = "$lib"; then
	    LIBL=""
	    break
	fi
    done
    if test -n "$LIBSW"; then
	$3="$$3 $LIBSW $LIBL"
    else
	$3="$$3 $LIBL"
    fi
    $4="$$4 $1"
])
dnl
dnl ------------------------------------------------------------------------
dnl Determine the dlname of a library to be installed by libtool
dnl     PL_GET_DLNAME(STEM,VERSION_INFO,VARIABLE)
dnl For a given library STEM and a given VERSION_INFO (a la
dnl -version-info option of libtool), determine the dlname of the
dnl library in the form lib$STEM.<so_ext>.<so_number>.  Set the
dnl variable VARIABLE with the resulting value.  This macro should be used
dnl only after the call to AM_PROG_LIBTOOL.
AC_DEFUN([PL_GET_DLNAME],[
  if test -z "$LIBTOOL" -a -z "$CC" ; then
    AC_MSG_ERROR([Dlname guessings can be done only after libtool is initialized])
  else
    AC_MSG_CHECKING([for name of shared library $1])  
    TMP_DIR=./tmp-cfg
    rm -rf $TMP_DIR
    mkdir -p $TMP_DIR
    cd $TMP_DIR
    echo "void f(void){}" > foo.c
    ../libtool --mode=compile $CC -c foo.c > /dev/null
    ../libtool --mode=link $CC foo.lo -rpath /usr/lib -version-info $2 \
        -o lib$1.la > /dev/null
    $3=`grep ^dlname= lib$1.la | sed "s/dlname='\(.*\)'/\1/"`
    cd ..
    rm -rf $TMP_DIR
    AC_MSG_RESULT([$[$3]])    
  fi
])
dnl ------------------------------------------------------------------------
dnl Determine the dlname of a DLL to be installed by libtool
dnl This is a usefule variation of GET_DLNAME above for dynamically loaded
dnl libraries (DLL's).
dnl     PL_GET_DLLNAME(STEM,VARIABLE)
dnl For a given DLL STEM determine the dlname of the
dnl library in the form $STEM.<so_ext>.  Set the
dnl variable VARIABLE with the resulting value.  This macro should be used
dnl only after the call to AM_PROG_LIBTOOL.
AC_DEFUN([PL_GET_DLLNAME],[
  if test -z "$LIBTOOL" -a -z "$CC" ; then
    AC_MSG_ERROR([Dlname guessings can be done only after libtool is initialized])
  else
    AC_MSG_CHECKING([for name of dynloaded module $1])
    TMP_DIR=./tmp-cfg
    rm -rf $TMP_DIR
    mkdir -p $TMP_DIR
    cd $TMP_DIR
    echo "void f(void){}" > foo.c
    ../libtool --mode=compile $CC -c foo.c > /dev/null
    ../libtool --mode=link $CC foo.lo -rpath /usr/lib -avoid-version -module \
        -o $1.la > /dev/null
    $2=`grep ^dlname= $1.la | sed "s/dlname='\(.*\)'/\1/"`
    cd ..
    rm -rf $TMP_DIR
    AC_MSG_RESULT([$[$2]])
  fi
])
dnl ------------------------------------------------------------------------
dnl PL_EXPAND_EXPRESSION(VARIABLE,EXPRESSION)
dnl
dnl Expand recursively a given EXPRESSION, until no variable expansion is
dnl possible.  Put the result in VARIABLE.
AC_DEFUN([PL_EXPAND_EXPRESSION],[
  str1="$2"
  while true ; do
   str2=`eval echo $str1`
     test "$str1" = "$str2" && break
     str1="$str2"
   done
   $1="$str2"
])
dnl ------------------------------------------------------------------------
dnl PL_OCTAVE_CONFIG_INFO(variable, config-field)
dnl
dnl Get Octave configuration values
dnl
AC_DEFUN([PL_OCTAVE_CONFIG_INFO], [
$1=`$OCTAVE -q -f <<EOF
        printf(octave_config_info("$2"));
EOF`])
dnl ------------------------------------------------------------------------
dnl PL_CONFIG_FILE_EXE(file)
dnl
dnl Add file to CONFIG_FILES list and make it executable
dnl
AC_DEFUN([PL_CONFIG_FILE_EXE],[
  AC_CONFIG_FILES([$1], [chmod +x $1])
])
dnl ------------------------------------------------------------------------
dnl PL_COMPARE_VERSIONS(version_string_1,version_string_2,
dnl       action_if_v1_lt_v2,action_if_v1_eq_v2,action_if_v1_gt_v2)
dnl
dnl Compare version strings, fields are separated by periods (".")
dnl
AC_DEFUN([PL_COMPARE_VERSIONS],[
  pl_verstr_1=$1
  pl_verstr_2=$2
  pl_cmp=eq
  while test -n "$pl_verstr_1" -o -n "$pl_verstr_2" ; do 
    pl_v1=`echo $pl_verstr_1 | cut -d. -f1`
    test -z "$pl_v1" && pl_v1=0
    pl_verstr_1=`echo $pl_verstr_1 | sed 's/[[^.]]\+.\?//'`
    pl_v2=`echo $pl_verstr_2 | cut -d. -f1`
    test -z "$pl_v2" && pl_v2=0
    pl_verstr_2=`echo $pl_verstr_2 | sed 's/[[^.]]\+.\?//'`
    if test "$pl_v1" -gt "$pl_v2" ; then
      pl_cmp=gt
      break;
    else 
      if test "$pl_v1" -lt "$pl_v2" ; then
        pl_cmp=lt
        break;
      fi
    fi
  done
  test $pl_cmp = lt && eval $3
  test $pl_cmp = eq && eval $4
  test $pl_cmp = gt && eval $5    
])

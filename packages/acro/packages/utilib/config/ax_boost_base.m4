# ===========================================================================
#          http://www.nongnu.org/autoconf-archive/ax_boost_base.html
# ===========================================================================
#
# SYNOPSIS
#
#   AX_BOOST_BASE( [MINIMUM-VERSION], 
#		   [available|optional|normal||required],
#		   [local boost snapshot | header location[, 
#                   object location ]] )
#
# DESCRIPTION
#
#   Test for the Boost C++ libraries of a particular version (or newer)
#
#   If no path to the installed boost library is given the macro searches
#   under /usr, /usr/local, /opt and /opt/local and evaluates the
#   $BOOST_ROOT environment variable. Further documentation is available at
#   <http://randspringer.de/boost/index.html>.
#
#   If the second argument controlls the "strictness" of the Boost
#   dependence:
#
#     []|[normal]: will raise an error if a suitable Boost version
#        is not found.
#
#     [optional]: will generate a warning instead of an error and
#        HAVE_BOOST will be set to false if a suitable Boost version 
#        is not found.
#
#     [required]: will raise an error if configure is called with
#        --with-boost=no.
#
#     [available]: HAVE_BOOST will be set to false and the probes 
#        will be skipped unless configure is explicitly called with
#        --with-boost.
#
#   This macro calls:
#
#     AC_SUBST(BOOST_CPPFLAGS) / AC_SUBST(BOOST_LDFLAGS)
#
#   And sets:
#
#     HAVE_BOOST / BUILD_BOOST_TPL
#
#   And declares the following environment variables `precious':
#
#     BOOST_ROOT / BOOST_LIB
#
# LICENSE
#
#   Copyright (c) 2008 Thomas Porschberg <thomas@randspringer.de>
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved.
#
#   This macro has been heavily modified from its original form for use
#   with Acro/DAKOTA.  Portions Copyright (c) 2009 Sandia Corporation.
#
#   This software is distributed under the BSD License.  
#   Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
#   the U.S. Government retains certain rights in this software.  For more
#   information, see the README.txt file in the top Acro directory.
#


AC_DEFUN([AX_BOOST_BASE_PROBE],
[
	CPPFLAGS_SAVED="$CPPFLAGS"
	LDFLAGS_SAVED="$LDFLAGS"

	AC_MSG_CHECKING([for boost >= $boost_lib_version_req [[$1]] ($2)])

	if test "$BOOST_CPPFLAGS" != ""; then
		CPPFLAGS="$CPPFLAGS_SAVED -I$BOOST_CPPFLAGS"
	fi
	if test "$BOOST_LDFLAGS" != ""; then
		LDFLAGS="$LDFLAGS_SAVED -L$BOOST_LDFLAGS"
	fi

	AC_LANG_PUSH(C++)
     	AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[
	@%:@include <boost/version.hpp>
	]], [[
	#if BOOST_VERSION >= $WANT_BOOST_VERSION
	// Everything is okay
	#else
	#  error Boost version is too old
	#endif
	]])],[
       	AC_MSG_RESULT([yes])
	BOOST_ROOT="$2"
	succeeded=yes
       	],[
       	AC_MSG_RESULT([no])
       	])
	AC_LANG_POP([C++])

	CPPFLAGS="$CPPFLAGS_SAVED"
	LDFLAGS="$LDFLAGS_SAVED"
])


AC_DEFUN([AX_BOOST_BASE],
[
AC_ARG_WITH([boost],
	AS_HELP_STRING([--with-boost@<:@=DIR@:>@], [use boost (default is yes) - it is possible to specify the root directory for boost (optional)]),
	[
	unset ac_boost_path
	ac_boost_path_src="(from --with-boost)"
	if test "$with_boost" = "no"; then
		want_boost="no"
	elif test "$with_boost" = "yes"; then
        	want_boost="yes"
		if test "${BOOST_ROOT+def}" = "def"; then
			ac_boost_path="$BOOST_ROOT"
			ac_boost_path_src="(from BOOST_ROOT)"
		fi
	else
		want_boost="yes"
	        ac_boost_path="$with_boost"
	fi
	],
	[
	dnl honor WITH_BOOST (including setting it to "no"), and BOOST_ROOT.
	dnl NB: be careful not to accidentally define them here if they don't
	dnl exist
	unset ac_boost_path
	ac_boost_path_src="(from --with-boost)"
	if test "${BOOST_ROOT+def}" = "def"; then
		want_boost="yes"
		ac_boost_path="$BOOST_ROOT"
		ac_boost_path_src="(from BOOST_ROOT)"
	dnl elif test "${WITH_BOOST+def}" = "def"; then
	dnl 	if test "$WITH_BOOST" = "no"; then
	dnl 		want_boost="no"
	dnl 	elif test "$WITH_BOOST" = "yes"; then
	dnl		want_boost="yes"
	dnl 	else
	dnl 		want_boost="yes"
	dnl 		ac_boost_path="$WITH_BOOST"
	dnl 	fi
	elif test "x$2" = "xavailable" ; then 
		want_boost="no"
	else
		want_boost="yes"
	fi
	])

AC_ARG_WITH([boost-libdir],
        AS_HELP_STRING([--with-boost-libdir=LIB_DIR],
        [Force given directory for boost libraries. Note that this will overwrite library path detection, so use this parameter only if default library detection fails and you know exactly where your boost libraries are located.]),
        [
	unset ac_boost_lib_path
	ac_boost_lib_path_src="(from --with-boost-libdir)"
        if test -d $withval; then
                ac_boost_lib_path="$withval"
        else
                AC_MSG_ERROR(--with-boost-libdir expected directory name)
        fi
        ],
        [
	unset ac_boost_lib_path
	ac_boost_lib_path_src="(from --with-boost-libdir)"
	if test "${BOOST_LIB+def}" = "def"; then
		ac_boost_lib_path="$BOOST_LIB"
		ac_boost_lib_path_src="(from BOOST_LIB)"
	fi
	]
)

dnl validate the dependence option
case "x$2" in
	x) ;;
	xnormal) ;;
	xavailable) ;;
	xoptional) ;;
	xrequired) ;;
	*) AC_MSG_ERROR([ax_boost_base: invalid option: $2 not in ([[available]], [[optional]], [[]], [[normal]], [[required]])]) ;;
esac

dnl Debugging info: notify if the root/lib paths were set explicitly
if test "${ac_boost_path+def}" = "def"; then
	AC_MSG_NOTICE([boost header path set to \"$ac_boost_path\" $ac_boost_path_src])
	case "$ac_boost_path" in
		"") ;;
		/*) ;;
		*) AC_MSG_WARN([specifying a relative path to boost likely problemmatic]) ;;
	esac
fi
if test "${ac_boost_lib_path+def}" = "def"; then
	AC_MSG_NOTICE([boost link library path set to \"$ac_boost_lib_path\" $ac_boost_lib_path_src])
	case "$ac_boost_path" in
		"") ;;
		/*) ;;
		*) AC_MSG_WARN([specifying a relative path to boost link libraries likely problemmatic]) ;;
	esac
fi

dnl Run through the Boost probes
if test "x$want_boost" = "xyes"; then
	pwd=`pwd`
	boost_system_search_paths="/usr /usr/local /opt /opt/local"

	boost_lib_version_req=ifelse([$1], ,1.20.0,$1)
	boost_lib_version_req_shorten=`expr $boost_lib_version_req : '\([[0-9]]*\.[[0-9]]*\)'`
	boost_lib_version_req_major=`expr $boost_lib_version_req : '\([[0-9]]*\)'`
	boost_lib_version_req_minor=`expr $boost_lib_version_req : '[[0-9]]*\.\([[0-9]]*\)'`
	boost_lib_version_req_sub_minor=`expr $boost_lib_version_req : '[[0-9]]*\.[[0-9]]*\.\([[0-9]]*\)'`
	if test "x$boost_lib_version_req_sub_minor" = "x" ; then
		boost_lib_version_req_sub_minor="0"
    	fi
	WANT_BOOST_VERSION=`expr $boost_lib_version_req_major \* 100000 \+  $boost_lib_version_req_minor \* 100 \+ $boost_lib_version_req_sub_minor`

	succeeded=no
	ac_boost_build_tpl=no

	dnl first check: is a sufficient version of Boost already on the search path???
	if test "${ac_boost_path-}" = "" -a "${ac_boost_lib_path-}" = ""; then
		dnl overwrite ld flags if we have required special directory with
    		dnl --with-boost-libdir parameter
		BOOST_CPPFLAGS=
		BOOST_LDFLAGS=
		AX_BOOST_BASE_PROBE([onpath], [])
	fi

	if test "$succeeded" != "yes" -a "${ac_boost_path-undef}" != ""; then
		dnl next we check common system locations for boost libraries
		dnl this location is chosen if boost libraries are installed 
		dnl with the --layout=system option or if you install boost with RPM
		with_boost=
		if test "${ac_boost_path+def}" = "def"; then
			with_boost="$ac_boost_path"
			BOOST_LDFLAGS="$ac_boost_path/lib"
			BOOST_CPPFLAGS="$ac_boost_path/include"
		else
			for ac_boost_path_tmp in $boost_system_search_paths ; do
				if test -d "$ac_boost_path_tmp/include/boost" && test -r "$ac_boost_path_tmp/include/boost"; then
					with_boost="$ac_boost_path_tmp"
					BOOST_LDFLAGS="$ac_boost_path_tmp/lib"
					BOOST_CPPFLAGS="$ac_boost_path_tmp/include"
					break;
				fi
			done
		fi

		dnl overwrite ld flags if we have required special directory with
    		dnl --with-boost-libdir parameter
	    	if test "${ac_boost_lib_path+def}" = "def"; then
       		    BOOST_LDFLAGS="$ac_boost_lib_path"
		fi

		if test -n "$with_boost"; then
			AX_BOOST_BASE_PROBE([system], [$with_boost])
		fi
	fi


	dnl if we found no boost with system layout we search for boost libraries
	dnl built and installed without the --layout=system option or for a staged(not installed) version
	if test "$succeeded" != "yes" -a "${ac_boost_path-undef}" != ""; then
		with_boost=
		_version=0
		if test "${ac_boost_path+def}" = "def"; then
			if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
				for i in `ls -d $ac_boost_path/include/boost-* 2>/dev/null`; do
					_version_tmp=`echo $i | sed "s#$ac_boost_path##" | sed 's/\/include\/boost-//' | sed 's/_/./'`
					V_CHECK=`expr $_version_tmp \> $_version`
					if test "$V_CHECK" = "1" ; then
						_version=$_version_tmp
					fi
					VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
					with_boost="$ac_boost_path"
					BOOST_CPPFLAGS="$ac_boost_path/include/boost-$VERSION_UNDERSCORE"
					BOOST_LDFLAGS="$ac_boost_path/stage/lib"
				done
			fi
		else
			for ac_boost_path_tmp in $boost_system_search_paths ; do
				if test -d "$ac_boost_path_tmp" && test -r "$ac_boost_path_tmp"; then
					for i in `ls -d $ac_boost_path_tmp/include/boost-* 2>/dev/null`; do
						_version_tmp=`echo $i | sed "s#$ac_boost_path_tmp##" | sed 's/\/include\/boost-//' | sed 's/_/./'`
						V_CHECK=`expr $_version_tmp \> $_version`
						if test "$V_CHECK" = "1" ; then
							_version=$_version_tmp
	               					best_path=$ac_boost_path_tmp
						fi
					done
				fi
			done

			VERSION_UNDERSCORE=`echo $_version | sed 's/\./_/'`
			with_boost="$best_path"
			BOOST_CPPFLAGS="$best_path/include/boost-$VERSION_UNDERSCORE"
			BOOST_LDFLAGS="$best_path/stage/lib"
		fi

		if test "${ac_boost_lib_path+def}" = "def"; then
			BOOST_LDFLAGS="$ac_boost_lib_path"
		fi

		if test -n "$with_boost"; then
			AX_BOOST_BASE_PROBE([staged], [$with_boost])
		fi
	fi

	dnl if we found no boost with system layout we search for boost libraries
	dnl bundled with this application (this is custom-spun for Acro/DAKOTA TPLs)
	if test "$succeeded" != "yes" -a "${ac_boost_path-undef}" != ""; then
		with_boost=
		unset best_tpl_path
		if test "${ac_boost_path+def}" = "def"; then
			if test -d "$ac_boost_path" && test -r "$ac_boost_path"; then
				if test -d "$ac_boost_path/boost"; then
					with_boost="$ac_boost_path"
					BOOST_CPPFLAGS="$with_boost"
					BOOST_LDFLAGS="$with_boost/stage/lib"
				fi
			fi
		elif test -n "$3"; then
			tpl_paths="$3"
			for ac_boost_path_tmp in $tpl_paths ; do
				if test -d "$ac_boost_path_tmp" && test -r "$ac_boost_path_tmp"; then
					if test -d "$ac_boost_path_tmp/boost"; then
						best_tpl_path="$ac_boost_path_tmp"
					fi
				fi
			done

			case "$best_tpl_path" in
				"") ;;
				/*) with_boost="$best_tpl_path" ;;
				*)  with_boost="$pwd/$best_tpl_path" ;;
			esac
			BOOST_CPPFLAGS="$with_boost"
			dnl The 4th parameter requires the presence & use of the 3rd: don't
			dnl allow the presence of a 4th parameter override --with-boost 
			dnl (here defined as ac_boost_path)
			case "$4" in
				"") BOOST_LDFLAGS="$with_boost/stage/lib" ;;
				/*) BOOST_LDFLAGS="$4/stage/lib" ;;
				*)  BOOST_LDFLAGS="$pwd/$4/stage/lib" ;;
			esac
		fi

		dnl if we have a separate library path (specified or
                dnl out-of-source build), override
		if test "${ac_boost_lib_path+def}" = "def"; then
			BOOST_LDFLAGS="${ac_boost_lib_path}"
		fi

		if test -n "$with_boost"; then
			AX_BOOST_BASE_PROBE([source], [$with_boost])
			dnl NB: bundled TPLs must be included as part of the autoconf build
			dnl by the sponsoring package -- as AC_CONFIG_SUBDIRS must be called 
			dnl with a literal path, the caller must check ac_boost_build_tpl 
			dnl and call AC_CONFIG_SUBDIRS themselves
			if test "${best_tpl_path-}" != "" -a "$succeeded" = "yes"; then
				ac_boost_build_tpl=yes
			fi
		fi
	fi
fi

if test "$succeeded" = "yes" -a "$want_boost" = "yes"; then
	case "$BOOST_CPPFLAGS" in
		"") ;;
		/*) BOOST_CPPFLAGS="-I$BOOST_CPPFLAGS" ;;
		 *) BOOST_CPPFLAGS="-I$BOOST_CPPFLAGS" ;
		    AC_MSG_WARN([BOOST_CPPFLAGS being set to a relative path ($BOOST_CPPFLAGS).]) ;
		    AC_MSG_WARN([   This likely indicates problems with ax_boost_base.m4]) ;;
	esac
	case "$BOOST_LDFLAGS" in
		"") ;;
		/*) BOOST_LIB="$BOOST_LDFLAGS";
		    BOOST_LDFLAGS="-L$BOOST_LDFLAGS" ;;
		 *) BOOST_LIB="$BOOST_LDFLAGS";
		    BOOST_LDFLAGS="-L$BOOST_LDFLAGS" ;
		    AC_MSG_WARN([BOOST_LDFLAGS being set to a relative path ($BOOST_LDFLAGS).]) ;
		    AC_MSG_WARN([   This likely indicates problems with ax_boost_base.m4]) ;;
	esac
	case "$BOOST_ROOT" in
		"")  ;;
		/*)  ;;
		 *) AC_MSG_WARN([BOOST_ROOT being set to a relative path ($BOOST_ROOT).]) ;
		    AC_MSG_WARN([   This likely indicates problems with ax_boost_base.m4]) ;;
	esac
	#AC_MSG_NOTICE([configuring without boost support: BOOST_ROOT     = $BOOST_ROOT])
	#AC_MSG_NOTICE([configuring without boost support: BOOST_CPPFLAGS = $BOOST_CPPFLAGS])
	#AC_MSG_NOTICE([configuring without boost support: BOOST_LDFLAGS  = $BOOST_LDFLAGS])
	AC_SUBST(BOOST_CPPFLAGS)
	AC_SUBST(BOOST_LDFLAGS)
	AC_DEFINE(HAVE_BOOST,,[define if the Boost library is available])
	export BOOST_ROOT
else
	with_boost=no
	if test "x$want_boost" = "xyes"; then
		if test "x$2" = "xoptional" ; then
			AC_MSG_WARN([No suitable boost library found (version $boost_lib_version_req_shorten or higher).])
			AC_MSG_NOTICE([configuring without boost support])
		else
			AC_MSG_ERROR([[We could not detect the boost libraries (version $boost_lib_version_req_shorten or higher). If you are sure you have boost installed, then check your version number looking in <boost/version.hpp>.]])
		fi
	elif test "x$2" = "xrequired"; then
		AC_MSG_ERROR([Invalid configuration option: --with-boost=no provided, but boost support is required.])
	else
		AC_MSG_NOTICE([configuring without boost support])
	fi
fi

export BOOST_LIB
dnl AC_ARG_VAR(WITH_BOOST, [Equivalent to --with-boost])
AC_ARG_VAR(BOOST_ROOT, [Path to Boost, a C++ foundation package])
AC_ARG_VAR(BOOST_LIB,  [Path to Boost link libraries, a C++ foundation package])

AM_CONDITIONAL([HAVE_BOOST], test "$succeeded" = "yes" -a "$want_boost" = "yes")
AM_CONDITIONAL([BUILD_BOOST_TPL], test "$ac_boost_build_tpl" = "yes")

])

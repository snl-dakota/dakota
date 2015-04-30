dnl @synopsis ACX_TINYXML([DEFAULT_SRC[, DEFAULT_BUILD[, ACTION-IF-FOUND[, ACTION-IF-NOT-FOUND]]]])
dnl
dnl This macro enables configuration of the TinyXML library.
dnl 
dnl In order, prefers (1) specified location, (2) TINYXML_SRC/BUILD,
dnl (3) DEFAULT_SRC/BUILD, (4) on path 
dnl
dnl This macro should check for a tinyxml installation in the system
dnl include directories.  But that is uncommon right now...
dnl

AC_DEFUN([ACX_TINYXML], [
acx_tinyxml_ok=no
acx_tinyxml_src_path=
acx_tinyxml_src_path_src=
acx_tinyxml_build_path=
acx_tinyxml_build_path_src=
dnl If $1 exists, use as default src location
acx_tinyxml_src_default=ifelse([$1], ,,$1)
dnl If $2 exists, use as default build location, else same as src path
acx_tinyxml_build_default=ifelse([$2],,[$acx_tinyxml_src_default],$2)
dnl Cannot check for build dir at configure time
if test ! -d "$acx_tinyxml_src_default"; then
	acx_tinyxml_src_default=
fi

AC_ARG_WITH(tinyxml,
	AS_HELP_STRING([--with-tinyxml=<dir>], [use TinyXML library in directory <dir>]),
	[
	case $with_tinyxml in
		yes | "") 
			acx_tinyxml_ok=yes ; 
	                if test "${TINYXML_SRC+def}" = "def" -a "${TINYXML_BUILD+def}" = "def"; then
				acx_tinyxml_src_path="$TINYXML_SRC"
				acx_tinyxml_src_path_src="(from TINYXML_SRC)"
				acx_tinyxml_build_path="$TINYXML_BUILD"
				acx_tinyxml_build_path_src="(from TINYXML_BUILD)"
			elif test "$acx_tinyxml_src_default" != "" -a "$acx_tinyxml_build_default" != ""; then
				acx_tinyxml_src_path="$acx_tinyxml_src_default"
				acx_tinyxml_src_path_src="(from configure.ac)"
				acx_tinyxml_build_path="$acx_tinyxml_build_default"
				acx_tinyxml_build_path_src="(from configure.ac)"
			else
				acx_tinyxml_src_path_src="(on path)"
				acx_tinyxml_build_path_src="(on path)"
			fi
			;;
		no) 
			acx_tinyxml_ok=disable 
			;;
		*) 
			acx_tinyxml_ok=yes ;
			acx_tinyxml_src_path="$with_tinyxml"
			acx_tinyxml_src_path_src="(from --with-tinyxml)"
			acx_tinyxml_build_path="$with_tinyxml"
			acx_tinyxml_build_path_src="(from --with-tinyxml)"
			;;
	esac
	],
	[
	dnl honor TINYXML_SRC/BUILD NB: be careful not to accidentally define 
	dnl them here if they don't exist
	if test "${TINYXML_SRC+def}" = "def" -a "${TINYXML_BUILD+def}" = "def"; then
		acx_tinyxml_ok=yes ; 
		acx_tinyxml_src_path="$TINYXML_SRC"
		acx_tinyxml_src_path_src="(from TINYXML_SRC)"
		acx_tinyxml_build_path="$TINYXML_BUILD"
		acx_tinyxml_build_path_src="(from TINYXML_BUILD)"
	elif test "$acx_tinyxml_src_default" != "" -a "$acx_tinyxml_build_default" != ""; then
		acx_tinyxml_ok=yes ; 
		acx_tinyxml_src_path="$acx_tinyxml_src_default"
		acx_tinyxml_src_path_src="(from configure.ac)"
		acx_tinyxml_build_path="$acx_tinyxml_build_default"
		acx_tinyxml_build_path_src="(from configure.ac)"
	fi
	])

TINYXML_LIB=
TINYXML_INCLUDE=
if test x"$acx_tinyxml_ok" = xyes; then

	case "$acx_tinyxml_src_path" in
		"") ;;
		/*) TINYXML_INCLUDE="-I$acx_tinyxml_src_path"
		    ;;
		*)  acx_tinyxml_src_path="`pwd`/$acx_tinyxml_src_path"
		    TINYXML_INCLUDE="-I$acx_tinyxml_src_path"
		    ;;
	esac

	AC_MSG_NOTICE([tinyxml src set to \"$acx_tinyxml_src_path\" $acx_tinyxml_src_path_src])
	TINYXML_SRC="$acx_tinyxml_src_path"
	export TINYXML_SRC


	case "$acx_tinyxml_build_path" in
		"") TINYXML_LIB="-ltinyxml"
		    ;;
		/*) TINYXML_LIB="-L$acx_tinyxml_build_path -ltinyxml"
		    ;;
		*)  acx_tinyxml_build_path="`pwd`/$acx_tinyxml_build_path"
		    TINYXML_LIB="-L$acx_tinyxml_build_path -ltinyxml"
		    ;;
	esac

	AC_MSG_NOTICE([tinyxml build set to \"$acx_tinyxml_build_path\" $acx_tinyxml_build_path_src])
	TINYXML_BUILD="$acx_tinyxml_build_path"
	export TINYXML_BUILD
fi

AC_ARG_VAR(TINYXML_SRC, [Path to TinyXML sources])
AC_ARG_VAR(TINYXML_BUILD, [Path to TinyXML objects])
AC_SUBST(TINYXML_INCLUDE)
AC_SUBST(TINYXML_LIB)

# Finally, execute ACTION-IF-FOUND/ACTION-IF-NOT-FOUND:
if test x"$acx_tinyxml_ok" = xyes; then
	AC_MSG_NOTICE([configuring with tinyxml support])
        ifelse([$3],,AC_DEFINE(HAVE_TINYXML,1,[Define if you have a TINYXML library.]),[$3])
        :
else
        acx_tinyxml_ok=no
	AC_MSG_NOTICE([configuring without tinyxml support])
        $4
fi
])dnl ACX_TINYXML

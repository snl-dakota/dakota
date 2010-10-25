dnl @synopsis AC_LATEX_PACKAGES([<package1>,<package2>,<package3>],<class>,<variable>)
dnl
dnl This macro test if package1 in <class> exists and if not package2
dnl and so and set <variable> to the right value
dnl
dnl  AC_LATEX_PACKAGES([allo,varioref,bonjour],book,vbook)
dnl  should set $vbook="varioref"
dnl
dnl @category LaTeX
dnl @author Mathieu Boretti <boretti@bss-network.com>
dnl @version 2005-12-18
dnl @license GPLWithACException

define(_AC_LATEX_PACKAGE_INTERNE,[
	ifelse($#,0,[],$#,1,[],$#,2,[],$#,3,[
		AC_LATEX_PACKAGE($3,$2,$1)
	],[
		AC_LATEX_PACKAGE($3,$2,$1)
		if test "$$1" = "yes";
		then
			$1=$3 ; export $1 ;
		else
			_AC_LATEX_PACKAGE_INTERNE($1,$2,m4_shift(m4_shift(m4_shift($@))))
		fi;
	])
])

AC_DEFUN([AC_LATEX_PACKAGES],[
	_AC_LATEX_PACKAGE_INTERNE($3,$2,$1)
	AC_SUBST($3)
])

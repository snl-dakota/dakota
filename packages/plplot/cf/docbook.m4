dnl cf/docbook.m4 for PLplot
dnl
dnl Autoconf macros for the DocBook documentation of PLplot
dnl
dnl Copyright (C) 2002, 2003, 2004  Alan W. Irwin
dnl Copyright (C) 2003, 2004  Rafael Laboissiere
dnl
dnl This file is part of PLplot.
dnl
dnl PLplot is free software; you can redistribute it and/or modify
dnl it under the terms of the GNU Library General Public License as
dnl published by the Free Software Foundation; version 2 of the License.
dnl
dnl PLplot is distributed in the hope that it will be useful,
dnl but WITHOUT ANY WARRANTY; without even the implied warranty of
dnl MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
dnl GNU Library General Public License for more details.
dnl
dnl You should have received a copy of the GNU Library General Public License
dnl along with PLplot; if not, write to the Free Software
dnl Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA

dnl ### AC_INIT(src/plplotdoc.xml.in)
dnl ### AM_INIT_AUTOMAKE(plplotdoc, 0.4.3)

dnl Utility macros

dnl CHECK_PROG(program, url)
AC_DEFUN([CHECK_PROG], [

pushdef([PROG], translit($1, [a-z], [A-Z]))

AC_ARG_WITH($1,
  AC_HELP_STRING([--with-$1=PATH/]PROG, [Alternative $1 program]),
  [prog_$1=$withval],
  [prog_$1=$1])
AC_CHECK_PROG(has_$1, [$prog_$1], found, no)
if test $has_$1 = no ; then
  PROG=
  if test -n "$2" ; then
    for i in "$2" ; do
      export $i=""
    done
  fi
else
  PROG=$prog_$1
fi
AC_SUBST(PROG)
AC_OUTPUT_COMMANDS( [
if test $has_$1 = no ; then]
AC_MSG_WARN( [
Program $1 not found.
    Install the $1 program in your path or specify its
    location with the option --with-$1.
    The $1 package can be found at
    $3.])
[fi], has_$1=$has_$1)
popdef([PROG])
])

dnl CHECK_DTD(title, cache-id, dsssl_dtd, docbookb_ss_dtd, style_spec_use,
dnl           external_specification, docbook_dtd, jade_output_type,
dnl           origin_package)
dnl ### CONFTEST=conftest
CONFTEST=doc/docbook/conftest
AC_SUBST(CONFTEST)
AC_DEFUN([CHECK_DTD], [
AC_CACHE_CHECK($1, pldb_cv_$2, [
echo -e "\nChecking for $1" >> $jadelog
sed -e "s|@DSSSL_DTD@|\"$DSSSL_DTD_PUBID\"|" -e "s|@DB_STYLE_SHEET@|$3|" \
  -e "s|@STYLE_SPEC_USE@|$4|" -e "s|@EXTERNAL_SPECIFICATION@|$5|" \
dnl ###  -e "s|@CONFTEST@|$CONFTEST|" < cnf/test.dsl.in > $CONFTEST.dsl
  -e "s|@CONFTEST@|$CONFTEST|" < doc/docbook/cnf/test.dsl.in > $CONFTEST.dsl
dnl ### sed "s|@DOCBOOK_DTD@|$6|" < cnf/test.xml.in > $CONFTEST.xml
sed "s|@DOCBOOK_DTD@|$6|" < doc/docbook/cnf/test.xml.in > $CONFTEST.xml

$prog_openjade $SGML_CATALOGS -d $CONFTEST.dsl -t $7 -o $CONFTEST.out \
   $XML_DECL $CONFTEST.xml > $jadelog.x 2>&1
jade_result=`egrep ":E:" $jadelog.x`
cat $jadelog.x >> $jadelog
rm -f $jadelog.x
if test "$jade_result" = "" ; then
  pldb_cv_$2=found
else
  pldb_cv_$2=no
fi
])
if test $pldb_cv_$2 = no ; then
  for i in $11 ; do
    export $i=""
  done
fi
AC_OUTPUT_COMMANDS([
if test $pldb_cv_$2 = no ; then]
  AC_MSG_WARN([Could not find $1.
    The following SGML public identifier could not be found in any of
    the default SGML catalogs:
        $dtd_$2
    Its system identifier[,] i.e. a file usually called
        \"$9\"
    is distributed with the $10 package.  Install it in your
    system and put the correct entry in the catalog file.
    You might also use the configure option --with-sgml-catalog to specify
    alternative SGML catalogs.])
[fi
], [
pldb_cv_$2=$pldb_cv_$2
dtd_$2="$8"
])
])

dnl File extensions

AC_DEFUN([FILE_EXT], [
pushdef([FILE], translit($1, [a-z], [A-Z]))
FILE[_EXT=$1]
AC_ARG_WITH($1-extension,
  AC_HELP_STRING([--with-$1-extension=EXT],
    [File extension for the generated FILE files. (Default value: $1)]),
  [FILE[_EXT=$withval]])
AC_SUBST(FILE[_EXT])
popdef([FILE])
])

dnl Info building

AC_DEFUN([CHECK_PERL_SCRIPT], [

pushdef([PROG], translit($1, [a-z], [A-Z]))

AC_ARG_WITH($1,
  AC_HELP_STRING([--with-$1=PATH/]PROG, [Alternative $1 Perl script]),
  [prog_$1=$withval],
  [prog_$1=/usr/bin/$1])
AC_MSG_CHECKING([for Perl script $1])
if test -x $prog_$1 ; then
  has_$1=found
else
  has_$1=no
fi
AC_MSG_RESULT([$has_$1])
PROG=$prog_$1
AC_SUBST(PROG)
if test $has_$1 = no ; then
  for i in $2 ; do
    export $i=""
  done
fi
AC_OUTPUT_COMMANDS( [
if test $has_$1 = no ; then]
AC_MSG_WARN( [
Program $1 not found.
    Install the perl script $1 program in your system or specify its
    location with the option --with-$1.
    The $1 package can be found at
    $3.])
[fi], has_$1=$has_$1)

popdef([PROG])
])

AC_DEFUN([CHECK_PM], [
AC_CACHE_CHECK( $1, pldb_cv_$2, [
cat > conftest.pl << EOF
use $1;
EOF
if test -n "$prog_perl" && $prog_perl conftest.pl >/dev/null 2>&1 ; then
  pldb_cv_$2=yes
else
  pldb_cv_$2=no
fi])
if test $pldb_cv_$2 = no ; then
  for i in $3; do
    export $i=""
  done
fi
AC_OUTPUT_COMMANDS([
if test $pldb_cv_$2 = no ; then]
  AC_MSG_WARN([
Could not find Perl module $1.
    Install it in your system at a place where Perl can find
    it (through @INC).])
[fi], [
pldb_cv_$2=$pldb_cv_$2
])
])

dnl Output commands

AC_DEFUN([DOC_OUTPUT_COMMANDS], [
AC_OUTPUT_COMMANDS( [
if test "$print" = "" ; then
  echo
  echo '  ******************************************************************'
  echo '  * Due to some lacking dependencies (see warning messages above), *'
  echo '  * it will not be possible to build a printed version of the      *'
  echo '  * documentation.                                                 *'
  echo '  ******************************************************************'
fi
if test "$html" = "" ; then
  echo
  echo '  ******************************************************************'
  echo '  * Due to some lacking dependencies (see warning messages above), *'
  echo '  * it will not be possible to build a HTML version of the         *'
  echo '  * documentation.                                                 *'
  echo '  ******************************************************************'
fi
if test "$info" = "" ; then
  echo
  echo '  ******************************************************************'
  echo '  * Due to some lacking dependencies (see warning messages above), *'
  echo '  * it will not be possible to build an info version of the        *'
  echo '  * documentation.                                                 *'
  echo '  ******************************************************************'
fi
if test "$man" = "" ; then
  echo
  echo '  ******************************************************************'
  echo '  * Due to some lacking dependencies (see warning messages above), *'
  echo '  * it will not be possible to build the man pages of the API      *'
  echo '  * chapter.                                                       *'
  echo '  ******************************************************************'
fi], [
print=$PRINT; html=$HTML; info=$INFO; man=$MAN
])
])

dnl ### AC_OUTPUT([Makefile
dnl ###          src/Makefile src/plplotdoc.xml src/plplotdoc-html.dsl
dnl ###          src/plplotdoc-print.dsl
dnl ###          bin/Makefile bin/api2man.pl
dnl ###          www/index.html.in])

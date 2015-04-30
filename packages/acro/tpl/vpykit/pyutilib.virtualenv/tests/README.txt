NOTE:  It's somewhat difficult to validate the behavior of the vpy_install script,
so the following tests describe checks that need to be performed manually.

UNIX

1. ../scripts/vpy_install

. Creates a ./python directory
. No source in python/src
. Installs setuptools (e.g. python/bin/easy_install exists)


2. ../scripts/vpy_install --trunk

. SAME AS 1


3.  lbin ../scripts/vpy_install --preinstall

. The directory python/dist/setuptools exists
. The directory python/lib does not exist
. The directory python/src is empty
. The file python/python.zip exists and contains the entire directory tree


4. ../scripts/vpy_install --offline --zip python/python.zip foo

NOTE: this uses the ZIP file generated in (3).

. Verify that this generates the same results as (1) by installing in a separate 
  directory and then diff'ing:

  ../scripts/vpy_install bar
  diff -r -x '*pyc' -x '*svn*' -x '*egg' foo bar


5. ../scripts/vpy_install --config test5.ini
   ../scripts/vpy_install --config test5.ini --trunk foo

. Verify that pyutilib.th is checked out in the python/dist and foo/dist directories
. Verify that pyutilib.enum is checked out in the python/src and foo/src directories
. Verify that tevaspot is checked out in the python/src and foo/src directories
. Verify differences using

  diff -w -x '.*svn' -r foo python | grep '^diff' | grep CHANGELOG

  . */dist/pyutilib.th/CHANGELOG.txt differs
  . */src/pyutilib.enum/CHANGELOG.txt differs

. Verify that the only complete differences are build, dist, *egg or *link files:

  diff -w -x '.*svn' -r foo python | grep '^Only'

. Verify that */dist/pyutilib.th contains build and dist directories for both

. Verify that neither *src/pyutilib.enum directory contains build and dist directories

. Verify that */doc exists


11. setenv SSG_USERNAME foobar
    ../scripts/vpy_install --config test5.ini

. Verify that pyutilib.plugin.core is checked out in python/src
. Verify that the 'foobar' user does not have commit priviledges


12. lbin ../scripts/vpy_install --preinstall --config test5.ini foo
    unsetenv HTTP_PROXY
    ../scripts/vpy_install --offline --zip foo/python.zip --config test5.ini bar

. Confirm that offline the installation works correctly without
  the HTTP_PROXY set.

#!/usr/bin/env python
# $Jeannot: setup.py,v 1.3 2004/04/18 00:26:32 js Exp $
#
# COBYLA python module setup
# To install the module in the current directory, use:
#  python setup.py build_ext --inplace
# To install globaly, use:
#  python setup.py install

from distutils.core import setup, Extension

module1 = Extension('moduleCobyla', sources = ['cobyla.c', 'moduleCobyla.c'])

setup (name = 'cobyla',
       version = '1.1',
       description = 'Michael J.D. Powell''s non-linear derivative free constrained optimization by linear approximation package',
       py_modules = ['cobyla'],
       ext_modules = [module1],
       author="Michael J.D. Powell (COBYLA), Jean-Sebastien Roy (Python interface)",
       author_email="js@jeannot.org",
       url="http://www.jeannot.org/~js/code/index.en.html#COBYLA")

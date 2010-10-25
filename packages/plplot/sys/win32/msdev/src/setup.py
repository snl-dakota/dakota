#!/usr/bin/env python
#
# Author: O. Svensson (svensson@esrf.fr)
#
# This is the setup.py file for building the python modules
# for the win32/msdev port of plplot. Use the command
#
#   python setup.py build_ext --inplace
#
# for creating the plplotc and plplot_widget modules.
# 
# I have borrowed a large part of this file from
# plplot/cf/setup.py.in, and the class MYbuild has been provided 
# by Gary Bishop (gb@cs.unc.edu).
#

import os, sys
from distutils.sysconfig import get_python_inc
from distutils.core import setup, Extension
from distutils.command.build_ext import build_ext

numeric_incdir = os.path.join(get_python_inc(plat_specific=1), "Numeric")

include_dirs       = ['.', '..\\tmp', numeric_incdir]
extra_compile_args = ['/DWIN32']
library_dirs       = ['..\\lib']
libraries          = ['plplot', 'kernel32', 'user32', 'gdi32', 'winspool', \
                      'comdlg32', 'advapi32', 'shell32', 'ole32', 'oleaut32', \
                      'uuid', 'odbc32', 'odbccp32']

# I'm overriding swig_sources in build_ext so that I can change 2 things about it behavior:
# 1) I want to be able to pass different arguments to swig
# 2) I want to post-process the swig output to fixup the doc strings
# Otherwise this is a copy of what I got from the docutils source
class MYbuild_ext(build_ext):
    def swig_sources (self, sources):

        """Walk the list of source files in 'sources', looking for SWIG
        interface (.i) files.  Run SWIG on all that are found, and
        return a modified 'sources' list with SWIG source files replaced
        by the generated C (or C++) files.
        """

        new_sources = []
        swig_sources = []
        swig_targets = {}

        # XXX this drops generated C/C++ files into the source tree, which
        # is fine for developers who want to distribute the generated
        # source -- but there should be an option to put SWIG output in
        # the temp dir.

        if self.swig_cpp:
            target_ext = '.cpp'
        else:
            target_ext = '.c'

        for source in sources:
            (base, ext) = os.path.splitext(source)
            if ext == ".i":             # SWIG interface file
                new_sources.append(base + target_ext)
                swig_sources.append(source)
                swig_targets[source] = new_sources[-1]
            else:
                new_sources.append(source)

        if not swig_sources:
            return new_sources

        swig = self.find_swig()
        swig_cmd = [swig, "-python", "-ISWIG", "-DPL_DOUBLE"]
        if self.swig_cpp:
            swig_cmd.append("-c++")

        for source in swig_sources:
            target = swig_targets[source]
            self.announce("swigging %s to %s" % (source, target))
            (base, ext) = os.path.splitext(target)
            interm = base + '_p' + ext
            self.spawn(swig_cmd + ["-o", interm, source])
            self.spawn(['python', 'makedocstrings.py', interm, target])

        return new_sources



module1 = Extension( "plplotc", 
		     extra_compile_args = extra_compile_args,
                     include_dirs = include_dirs,
		     library_dirs = library_dirs,
		     libraries = libraries,
		     sources = ["plplotcmodule.i"],
                   )

module2 = Extension( "plplot_widget", 
		     extra_compile_args = extra_compile_args,
                     include_dirs = include_dirs,
		     library_dirs = library_dirs,
		     libraries = libraries,
		     sources = ["plplot_widgetmodule.c"]
		     )

setup( name = "plplot",
       version = "5.1",
       description = "PLplot Python extension modules",
       author = "Alan W. Irwin",
       author_email = "irwin@beluga.phys.uvic.ca",
       url = "http://www.plplot.org",
       license = "LGPL",
       ext_modules = [module1,module2],
       data_files = [("..\\lib", ['plstnd5.fnt', 'plxtnd5.fnt'])],
       cmdclass = {'build_ext': MYbuild_ext},		     
       )

#!/bin/sh
# Fix up a clean exported plplot tree in order to generate a release tarball.
# Must be executed from top directory of exported tree, typically a versioned
# directory such as plplot-5.2.0.
# Usage:
# If autotools installed in normal system locations:
# scripts/make_tarball.sh
# If, for example, autotools installed in /home/software/autotools/install 
# scripts/make_tarball.sh -I /home/software/autotools/install/share/libtool/libltdl

# Add in generated documentation from our website.
#cd doc
#wget -r -l 1 -L -nH --no-parent --cut-dirs=2 \
#http://plplot.sourceforge.net/resources/docbook-manual/
#tar zxf plplotdoc-html-0.4.3.tar.gz
#cd ..

# Fix permissions screwup(s) in freshly checked out CVS
# Nothing to do for the moment thanks to Rafael's efforts

# Prepare tree for configure; make; make install

./bootstrap.sh 

# Compensate for autoconf bug which invokes autoheader inappropriately on these
# files if they are out of date.
cd include
touch plConfig.h.in  plDevs.h.in
cd ..

# Perl-generate the tcl wrapper files.
cd bindings/tcl 
perl pltclgen
cd ../..

# Swig-generate both the single and double-precision python wrappers.
cd bindings/python
swig -python -o plplotcmodule_p_double.c -c++ -DPL_DOUBLE plplotcmodule.i
swig -python -o plplotcmodule_p_single.c -c++ plplotcmodule.i
cd ../..

# Swig-generate the java wrapper.
cd bindings/java
swig -java -package plplot.core -DPL_DOUBLE plplotjavac.i
cd ../..

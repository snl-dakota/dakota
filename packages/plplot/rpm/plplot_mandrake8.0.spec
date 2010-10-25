# Create rpm binary and source packages with ordinary user account using
# safe temporary install location with eventual root ownership of files that 
# are actually installed in the final system location by the binary rpm.

BuildRoot: /tmp/software/redhat_install_area/plplot
# N.B. note use of $RPM_BUILD_ROOT (which points to this location) in the 
# %prep and %install areas, and explicit removal of this location in 
# %install area before actual install to this location occurs.
Summary: PLPlot 2D/3D plotting library
Packager: Conrad Steenberg <conrad@hep.caltech.edu>
Name: plplot
Version: 5.0.4
Release: 3mdk
Source0: http://prdownloads.sourceforge.net/plplot/plplot-5.0.4.tar.gz
URL: http://plplot.sourceforge.net
Copyright: LGPL with some exceptions, see file "Copyright"
Group: Applications/Math
requires: python >= 1.5.2, python-numeric >= 15.3 
%description
This is the distribution for PLplot, a scientific plotting package. PLplot
is relatively small, portable, freely distributable, and is rich enough to
satisfy most users.  It has a wide range of plot types including line
(linear, log), contour, 3D, fill, and almost 1000 characters (including
Greek and mathematical) in its extended font set.  The package is designed
to make it easy to quickly get graphical output; only a handful of function
calls is typically required.  For more advanced use, virtually all aspects
of plotting are configurable.

Notes on the plplot configuration underlying this package for the
Mandrake build environment: 

(i) We use --with-double=yes to give double precision.

(ii) A large number of drivers are configured by default including tk, ps,
psc, png, jpeg, and we also explicitly configure --enable-gnome to
include the experimental gnome driver.

(iii) We explicitly configure --disable-octave, because octave is not part 
of the default distribution.

(iv) Languages supported are c, c++, fortran 77, python, and tcl. All these
packages are standard on Mandrake 8.0.

The full configure output is as follows:

with_shlib:     yes             with_double:    yes
with_debug:     no              with_opt:       yes
with_warn:      no              with_profile:   no
with_gcc:       yes             with_rpath:     yes

enable_xwin:    yes             enable_tcl:     yes
enable_tk:      yes             enable_itcl:    no
enable_cxx:     yes             enable_python:  yes
enable_f77:     yes             enable_gnome:   yes
enable_octave:  no

%prep
%setup
make configure
PY_VERSION=`python -c 'import sys ; print sys.version[0:3]'`
export PYTHON_INC_DIR=/usr/include/python${PY_VERSION}/
echo PYTHON_INC_DIR=${PYTHON_INC_DIR}
export PYTHON_MOD_DIR=/usr/lib/python${PY_VERSION}/ 
export PYTHON_CFG_DIR=${PYTHON_MOD_DIR}/config
export PYTHON_NUM_DIR=${PYTHON_INC_DIR}/Numeric/
export PYTHON_MACH_DIR=${PYTHON_MOD_DIR}/site-packages
export PYTHON_DIR=${PYTHON_MACH_DIR}
./configure --prefix=/usr --with-double --disable-octave --enable-gnome --enable-python --enable-tcl --disable-itcl
%build
make 
%install
# remove safe install area before installing to it, but be explicit 
# because $RPM_BUILD_ROOT (which is set to this area within this script)
# can be overridden in error from outside
rm -rf /tmp/software/redhat_install_area/plplot
# The PREFIX value makes this package relocatable.  So all compilation was
# done with --prefix=/usr, but the install occurs to PREFIX (temporarily)
# until the binary rpm is put together.
make install PREFIX=$RPM_BUILD_ROOT/usr
# hack to get rid of these non-working examples (that screw things up in any
# case because they start with #!/usr/local/bin/python which adds a wrong
# dependency)
rm -f $RPM_BUILD_ROOT/usr/share/doc/plplot/examples/python/tutor.py
rm -f $RPM_BUILD_ROOT/usr/share/doc/plplot/examples/python/x??.py
# install extra documentation
cd doc
gzip plplotdoc*.dvi
cp plplotdoc*gz  index.html $RPM_BUILD_ROOT/usr/share/doc/plplot
gunzip plplotdoc*.dvi.gz
pushd $RPM_BUILD_ROOT/usr/share/doc/plplot
# * stands for version number of plplotdoc.
tar zxf plplotdoc-html-*.tar.gz
rm $RPM_BUILD_ROOT/usr/share/doc/plplot/plplotdoc-html-*.tar.gz
popd
# install info stuff.  This is the correct place for RH 6.2, but may not
# be the preferred location for modern distributions.
install -m 755 -d $RPM_BUILD_ROOT/usr/share/info
# * stands for version number of plplotdoc.
tar zxf plplotdoc-info-*.tar.gz
rm $RPM_BUILD_ROOT/usr/share/doc/plplot/plplotdoc-info-*.tar.gz
bzip2 plplotdoc-info-*/*
cp plplotdoc-info-*/* $RPM_BUILD_ROOT/usr/share/info
# make sure can redo this script in case of --short-circuit
rm -f plplotdoc-info-*/*
# install man pages
install -m 755 -d $RPM_BUILD_ROOT/usr/man/man1
cp plm2gif.1 plpr.1 plrender.1 plserver.1 pltcl.1 pltek.1 $RPM_BUILD_ROOT/usr/man/man1
pushd $RPM_BUILD_ROOT/usr/man/man1
bzip2 plm2gif.1 plpr.1 plrender.1 plserver.1 pltcl.1 pltek.1
popd
tar zxf plplotdoc-man-*.tar.gz
rm  $RPM_BUILD_ROOT/usr/share/doc/plplot/plplotdoc-man-*.tar.gz 
bzip2 plplotdoc-man-*/*
install -m 755 -d $RPM_BUILD_ROOT/usr/man/man3
cp plplotdoc-man-*/*.bz2 $RPM_BUILD_ROOT/usr/man/man3
# make sure can redo this script in case of --short-circuit
rm -f plplotdoc-man-*/*
cd ..
%post
/sbin/ldconfig
/sbin/install-info --entry="* PLplot: (plplotdoc).  PLplot plotting suite." /usr/share/info/plplotdoc.info.bz2 /usr/share/info/dir
%preun
if [ $1 = 0 ]; then
   /sbin/install-info --delete /usr/share/info/plplotdoc.info.bz2 /usr/share/info/dir
fi
%postun
/sbin/ldconfig
%files
# doc plus examples (for now)
%attr(-, root, root) %doc /usr/share/doc/plplot
%attr(-, root, root) %doc /usr/man/man1/plm2gif.1.bz2 
%attr(-, root, root) %doc /usr/man/man1/plpr.1.bz2 
%attr(-, root, root) %doc /usr/man/man1/plrender.1.bz2
%attr(-, root, root) %doc /usr/man/man1/plserver.1.bz2
%attr(-, root, root) %doc /usr/man/man1/pltcl.1.bz2
%attr(-, root, root) %doc /usr/man/man1/pltek.1.bz2 
%attr(-, root, root) %doc /usr/man/man3/*.3plplot.bz2 
# octave support files for Plplot.
#N.A. on Mdk8.0%attr(-, root, root) /usr/share/plplot-octave
# python module
%attr(-, root, root) /usr/lib/python*/site-packages/plmodule.so
# fonts and maps 
%attr(-, root, root) /usr/lib/plplot5.0.4
# info files
%attr(-, root, root) /usr/share/info/plplotdoc.info*.bz2
# headers
%attr(-, root, root) /usr/include/plplot
# executables
%attr(-, root, root) /usr/bin/plm2gif
%attr(-, root, root) /usr/bin/plplot-config
%attr(-, root, root) /usr/bin/plpr
%attr(-, root, root) /usr/bin/plrender
%attr(-, root, root) /usr/bin/plserver
%attr(-, root, root) /usr/bin/pltcl
%attr(-, root, root) /usr/bin/pltek
%attr(-, root, root) /usr/bin/pstex2eps
# libraries
%attr(-, root, root) /usr/lib/libplcxxd.*
%attr(-, root, root) /usr/lib/libplplotd.*
%attr(-, root, root) /usr/lib/libtclmatrixd.*

# Create rpm binary and source packages with ordinary user account using
# safe temporary install location with eventual root ownership of files that 
# are actually installed in the final system location by the binary rpm.

BuildRoot: /tmp/software/redhat_install_area/plplot
# N.B. note use of $RPM_BUILD_ROOT (which points to this location) in the 
# %prep and %install areas, and explicit removal of this location in 
# %install area before actual install to this location occurs.

%define version 5.1.0
%define rpmversion 1rh7.2

Summary: PLPlot 2D/3D plotting library
Packager: Alan W. Irwin <irwin@beluga.phys.uvic.ca>
Name: plplot
Version: %{version}
Release: %{rpmversion}
Source0: http://prdownloads.sourceforge.net/plplot/plplot-%{version}.tar.gz
URL: http://plplot.sourceforge.net
Copyright: LGPL with some exceptions, see file "Copyright"
Group: Applications/Math
requires: python >= 1.5.2-30, python-numpy >= 15.3, octave >= 2.1.34, matwrap >= 0.57
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
RH 7.2 build environment: 

(i) We use --with-double --enable-dyndrivers --enable-gnome --enable-ntk --disable-linuxvga
to give double precision, dynamic drivers, and the experimental gnome
and ntp drivers.  We exclude the linuxvga driver because the contributed
RedHat svgalib rpm is so outdated (1999), and it is not clear it will even
work for RH 7.2.  We do not enable java.

(ii) In addition, a large number of drivers are configured by default
including tk, ps, psc, png and jpeg.

(iii) The supported front ends are c, c++, fortran 77, python (with numpy),
tcl/tk, and octave.  The octave front end requires installation of the
matwrap rpm (available at http://prdownloads.sourceforge.net/plplot/)

The configuration summary is as follows:

CC:             gcc -c          -O  
LDC:            gcc    
CXX:            g++ -c            -O 
LDCXX:          g++      
F77:            f77 -c           -O 
LDF:            f77    
INCS:            -I. -I/usr/include/gtk-1.2 -I/usr/include/glib-1.2 -I/usr/lib/glib/include -I/usr/X11R6/include -I/usr/include -I/usr/lib/gnome-libs/include
LIBS:            -litk -ltk8.3 -litcl -ltcl8.3 -L/usr/X11R6/lib -lX11 -ldl -lm -lg2c
LIB_TAG:        d
devices:         plmeta null xterm tek4010 tek4010f tek4107 tek4107f mskermit conex vlt versaterm dg300 png jpeg ps psc xfig ljii hp7470 hp7580 lj_hpgl ljiip imp xwin tk pbm gnome pstex ntk

Available device drivers
static:          xwin tk
dynamic:         plmeta null tek dg300 gd ps xfig ljii hpgl ljiip impress pbm gnome pstex ntk

with_shlib:     yes             with_double:    yes
with_debug:     no              with_opt:       yes
with_warn:      no              with_profile:   no
with_gcc:       yes             with_rpath:     yes

enable_xwin:    yes             enable_tcl:     yes
enable_tk:      yes             enable_itcl:    yes
enable_cxx:     yes             enable_python:  yes
enable_f77:     yes             enable_java:    no
enable_octave:  yes             enable_gnome:   yes

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
./configure --prefix=/usr --with-double --enable-dyndrivers --enable-gnome --enable-ntk --disable-linuxvga

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
cp plplotdoc*.gz plplotdoc*.dvi index.html $RPM_BUILD_ROOT/usr/share/doc/plplot
pushd $RPM_BUILD_ROOT/usr/share/doc/plplot
# * stands for version number of plplotdoc.
tar zxf plplotdoc-html-*.tar.gz
popd

# install info stuff.
install -m 755 -d $RPM_BUILD_ROOT/usr/share/info
# * stands for version number of plplotdoc.
tar zxf plplotdoc-info-*.tar.gz
gzip plplotdoc-info-*/*
cp plplotdoc-info-*/* $RPM_BUILD_ROOT/usr/share/info

# make sure can redo this script in case of --short-circuit
rm -f plplotdoc-info-*/*

# install man pages
install -m 755 -d $RPM_BUILD_ROOT/usr/share/man/man1
cp plm2gif.1 plpr.1 plrender.1 plserver.1 pltcl.1 pltek.1 $RPM_BUILD_ROOT/usr/share/man/man1
pushd $RPM_BUILD_ROOT/usr/share/man/man1
gzip plm2gif.1 plpr.1 plrender.1 plserver.1 pltcl.1 pltek.1
popd
tar zxf plplotdoc-man-*.tar.gz
gzip plplotdoc-man-*/*
install -m 755 -d $RPM_BUILD_ROOT/usr/share/man/man3
cp plplotdoc-man-*/*.gz $RPM_BUILD_ROOT/usr/share/man/man3

# make sure can redo this script in case of --short-circuit
rm -f plplotdoc-man-*/*
cd ..

%post
/sbin/ldconfig
/sbin/install-info --entry="* PLplot: (plplotdoc).  PLplot plotting suite." /usr/share/info/plplotdoc.info.gz /usr/share/info/dir
%preun
if [ $1 = 0 ]; then
   /sbin/install-info --delete /usr/share/info/plplotdoc.info.gz /usr/share/info/dir
fi
%postun
/sbin/ldconfig
%files
# doc
%attr(-, root, root) %doc /usr/share/doc/plplot
%attr(-, root, root) %doc /usr/share/man/man1/plm2gif.1.gz 
%attr(-, root, root) %doc /usr/share/man/man1/plpr.1.gz 
%attr(-, root, root) %doc /usr/share/man/man1/plrender.1.gz 
%attr(-, root, root) %doc /usr/share/man/man1/plserver.1.gz 
%attr(-, root, root) %doc /usr/share/man/man1/pltcl.1.gz
%attr(-, root, root) %doc /usr/share/man/man1/pltek.1.gz 
%attr(-, root, root) %doc /usr/share/man/man3/*.3plplot.gz 
# octave support files for Plplot.
%attr(-, root, root) /usr/share/plplot_octave
# python modules
%attr(-, root, root) /usr/lib/python*/site-packages/plmodule.so
%attr(-, root, root) /usr/lib/python*/site-packages/pyqt_plmodule.so
# fonts, maps, tcl data, dyndrivers, and examples
%attr(-, root, root) /usr/lib/plplot%{version}
# info files
%attr(-, root, root) /usr/share/info/plplotdoc.info*.gz
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

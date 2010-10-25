# Create rpm binary and source packages with ordinary user account using
# safe temporary install location with eventual root ownership of files that 
# are actually installed in the final system location by the binary rpm.

BuildRoot: /tmp/software/redhat_install_area/plplot
# N.B. note use of $RPM_BUILD_ROOT (which points to this location) in the 
# %prep and %install areas, and explicit removal of this location in 
# %install area before actual install to this location occurs.

%define version 5.2.1.cvs.20030915
%define rpmversion 1rh7.3

Summary: PLPlot 2D/3D plotting library
Packager: Alan W. Irwin <irwin@beluga.phys.uvic.ca>
Name: plplot
Version: %{version}
Release: %{rpmversion}
Source0: http://prdownloads.sourceforge.net/plplot/plplot-%{version}.tar.gz
#Patch0: plplot-%{version}.patch
URL: http://plplot.sourceforge.net
Copyright: LGPL with some exceptions, see file "Copyright"
Group: Applications/Math
# N.B. specific version of octave required because of versioned install
# location.  Better way to handle this is to determine the octave versioned
# install location automatically (under rpm), but I don't know how to do
# that.
requires: python >= 1.5.2-30, python-numpy >= 15.3, octave = 2.1.35
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
RH 7.3 build environment: 

(i) We use 
--with-double --enable-dyndrivers --enable-octave --enable-gnome --enable-ntk --disable-linuxvga
to give double precision, dynamic drivers, and the experimental octave, gnome
and ntk drivers.  We exclude the linuxvga driver because the contributed
RedHat svgalib rpm is so outdated (1999), and it is not clear it will even
work for RH 7.3.  We do not enable java since that demands having system java
installed and apparently that is not possible on RedHat because of the
Sun licensing issue. Eventually, we will get around this with a gcc-libgcj
approach.

(ii) In addition, a large number of drivers are configured by default
including tk, ps, psc, png and jpeg.

(iii) The supported front ends are c, c++, fortran 77, python (with numpy),
tcl/tk, and octave.

Configure results:

command:	./configure --prefix=/usr --with-double --enable-dyndrivers --enable-octave --enable-gnome --enable-ntk --disable-linuxvga
system:		i686-pc-linux-gnu
prefix:		/usr
CC:		gcc 
CXX:		g++ 
F77:		g77 
LIB_TAG:	d
devices:	 dg300 png jpeg gnome hp7470 hp7580 lj_hpgl imp ljii ljiip ntk null pbm plmeta ps psc pstex xterm tek4010 tek4107 mskermit versaterm vlt conex tek4010f tek4107f tk xfig xwin

Available device drivers
static:		
dynamic:	 dg300d_drv.la gdd_drv.la gnomed_drv.la hpgld_drv.la impressd_drv.la ljiid_drv.la ljiipd_drv.la ntkd_drv.la nulld_drv.la pbmd_drv.la plmetad_drv.la psd_drv.la pstexd_drv.la tekd_drv.la tkd_drv.la xfigd_drv.la xwind_drv.la

with_shlib:	yes		with_double:	yes
with_debug:	no		with_opt:	yes
with_warn:	no		with_profile:	no
with_gcc:	yes		with_rpath:	yes
with_freetype:	no

enable_xwin:	yes		enable_tcl:	yes
enable_tk:	yes		enable_itcl:	no
enable_cxx:	yes		enable_python:	yes
enable_f77:	yes		enable_java:	no
enable_octave:	yes		enable_gnome:	yes
enable_tkwin:	no

%prep
%setup -q
#%patch -p1

# This workaround won't be needed for the next version!
PY_VERSION=`python -c 'import sys ; print sys.version[0:3]'`
export PYTHON_INC_DIR=/usr/include/python${PY_VERSION}/
echo PYTHON_INC_DIR=${PYTHON_INC_DIR}
./configure --prefix=/usr --disable-linuxvga

%build
make

%install
# remove safe install area before installing to it, but be explicit 
# because $RPM_BUILD_ROOT (which is set to this area within this script)
# can be overridden in error from outside
rm -rf /tmp/software/redhat_install_area/plplot

# We use the automake DESTDIR here so the pre-install occurs in
# $RPM_BUILD_ROOT/usr until the binary rpm is put together to actually
# install in /usr.
make DESTDIR=$RPM_BUILD_ROOT install

# install extra documentation
cd doc/docbook/src
cp plplot*.gz plplot*.dvi index.html $RPM_BUILD_ROOT/usr/share/doc/plplot

# install html stuff.
install -m 755 -d $RPM_BUILD_ROOT/usr/share/doc/plplot/html
cp `cat HTML-MANIFEST` stylesheet.css \
$RPM_BUILD_ROOT/usr/share/doc/plplot/html

# install info stuff.
install -m 755 -d $RPM_BUILD_ROOT/usr/share/info
# * stands for version number of plplot.
cp plplot*.info* $RPM_BUILD_ROOT/usr/share/info
pushd $RPM_BUILD_ROOT/usr/share/info
gzip plplot*.info*
popd

# install man pages
install -m 755 -d $RPM_BUILD_ROOT/usr/share/man/man1
pushd ../..
cp plm2gif.1 plpr.1 plrender.1 plserver.1 pltcl.1 pltek.1 $RPM_BUILD_ROOT/usr/share/man/man1
popd
pushd $RPM_BUILD_ROOT/usr/share/man/man1
gzip plm2gif.1 plpr.1 plrender.1 plserver.1 pltcl.1 pltek.1
popd
install -m 755 -d $RPM_BUILD_ROOT/usr/share/man/man3
cp *.3plplot $RPM_BUILD_ROOT/usr/share/man/man3
pushd $RPM_BUILD_ROOT/usr/share/man/man3
gzip *.3plplot
popd

cd ../../..

%post
/sbin/ldconfig
/sbin/install-info --entry="* PLplot: (plplotdoc).  PLplot plotting suite." \
/usr/share/info/plplotdoc.info.gz /usr/share/info/dir
%preun
if [ $1 = 0 ]; then
   /sbin/install-info --delete /usr/share/info/plplotdoc.info.gz \
   /usr/share/info/dir
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
%attr(-, root, root) /usr/libexec/octave/2.1.35/oct/i386-redhat-linux-gnu/plplot_octave.oct
# python modules
%attr(-, root, root) /usr/lib/python*/site-packages/_plplotcmodule.so
%attr(-, root, root) /usr/lib/python*/site-packages/plplot.py
%attr(-, root, root) /usr/lib/python*/site-packages/plplot_widgetmodule.so
%attr(-, root, root) /usr/lib/python*/site-packages/plplotc.py

# fonts, maps, tcl data, dyndrivers, and examples
%attr(-, root, root) /usr/lib/plplot%{version}
# info files
%attr(-, root, root) /usr/share/info/plplot*.info*.gz
# headers
%attr(-, root, root) /usr/include/plplot
# executables
%attr(-, root, root) /usr/bin/plm2gif
%attr(-, root, root) /usr/bin/plplot-config
%attr(-, root, root) /usr/bin/plplot_libtool
%attr(-, root, root) /usr/bin/plpr
%attr(-, root, root) /usr/bin/plrender
%attr(-, root, root) /usr/bin/plserver
%attr(-, root, root) /usr/bin/pltcl
%attr(-, root, root) /usr/bin/pltek
%attr(-, root, root) /usr/bin/pstex2eps
# libraries
%attr(-, root, root) /usr/lib/libcsa.*
# Only if system has libqhull 
# %attr(-, root, root) /usr/lib/libnn.*
%attr(-, root, root) /usr/lib/libplplotcxxd.*
%attr(-, root, root) /usr/lib/libplplotd.*
%attr(-, root, root) /usr/lib/libplplotf77d.*
%attr(-, root, root) /usr/lib/libplplottcltkd.*
%attr(-, root, root) /usr/lib/libtclmatrixd.*
# pkgconfig directory
%attr(-, root, root) /usr/lib/pkgconfig/plplot.pc

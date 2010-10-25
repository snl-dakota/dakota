# Create rpm binary and source packages with ordinary user account using
# safe temporary install location with eventual root ownership of files that 
# are actually installed in the final system location by the binary rpm.

BuildRoot: /tmp/software/redhat_install_area/plplot
# N.B. note use of $RPM_BUILD_ROOT (which points to this location) in the 
# %prep and %install areas, and explicit removal of this location in 
# %install area before actual install to this location occurs.
Summary: PLPlot 2D/3D plotting library
Packager: Alan W. Irwin <irwin@beluga.phys.uvic.ca>
Name: plplot
Version: 5.1.0
Release: 1
Source0: http://prdownloads.sourceforge.net/plplot/plplot-5.1.0.tar.gz
URL: http://plplot.sourceforge.net
Copyright: LGPL with some exceptions, see file "Copyright"
Group: Applications/Math
requires: python >= 1.5.2-30, python-numpy >= 15.3 
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
RH 7.1 build environment: 

(i) We use --with-double --enable-dyndrivers --enable-gnome --enable-ntk  
to give double precision, dynamic drivers, and the experimental gnome
and ntp drivers.  We do not enable java.

(ii) A large number of drivers are also configured by default including tk, ps,
psc, png and jpeg.

(iii) --enable-octave is configured by default, but octave is not supported
on RedHat 7.1 because of a missing matwrap package.  Thus, the configure
script automatically disables it, and to be consistent with that result we
must comment out the octave files from the file list.  To get around this
problem we will some day make a matwrap rpm (which should be easy to do
since no compilation is involved).  But not today!

%prep
%setup
make configure
./configure --prefix=/usr --with-double --enable-dyndrivers --enable-gnome --enable-ntk
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
# Not available because of matwrap%attr(-, root, root) /usr/share/plplot-octave
# python modules
%attr(-, root, root) /usr/lib/python1.5/site-packages/plmodule.so
%attr(-, root, root) /usr/lib/python1.5/site-packages/pyqt_plmodule.so
# fonts, maps, tcl data, dyndrivers, and examples
%attr(-, root, root) /usr/lib/plplot5.1.0
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

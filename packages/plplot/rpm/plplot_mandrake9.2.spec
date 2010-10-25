%define name    plplot
%define version 5.3.0
%define release 1mdk
%define lib_name_orig libplplot
%define lib_major 9
%define lib_name               libplplot9
%define lib_name_devel         libplplot9-devel
%define lib_name_static_devel  libplplot9-static-devel

# These are unnecessary
%define libname 		%mklibname plplot %{lib_major}
%define libnamedevel		%mklibname plplot %{lib_major} -d
%define libnamestaticdevel		%mklibname plplot %{lib_major} -s -d

Summary: Scientific 2D/3D plotting library
Packager: Brian D. Wright <bdwright@phy.ucsf.edu>
Name:    %{name}
Version: %{version}
Release: %{release}
Source0: http://prdownloads.sourceforge.net/plplot/%{name}-%{version}.tar.bz2
# This patch needed if --enable-linuxvga
Patch0:  plplot-5.2.1.cvs.20031030_vga.patch.bz2
BuildRoot: %{_tmppath}/%{name}-root
URL: 	http://plplot.sourceforge.net
Copyright: LGPL with some exceptions, see file "Copyright"
Group: Sciences/Mathematics
BuildRequires: libqhull-devel tcl tk itcl python-devel >= 2.1.1, python-numeric >= 20.1.0 java-devel freetype2-devel octave >= 2.1 glib-devel gtk-devel >= 1.2.7 libgnome-devel >= 1.4 libcd-devel swig >= 1.3
#BuildRequires: svgalib-devel
Requires: %{lib_name} = %{version}

%description
This is the distribution for PLplot, a scientific plotting package.  PLplot is
relatively small, portable, freely distributable, and is rich enough to
satisfy most users.  It has a wide range of plot types including line (linear,
log), contour, 3D, fill, and almost 1000 characters (including Greek and
mathematical) in its extended font set.  The package is designed to make it
easy to quickly get graphical output; only a handful of function calls is
typically required.  For more advanced use, virtually all aspects of plotting
are configurable.

The PLplot library is written in C, enabling it to run on many platforms
practically without modification.  Fortran programs may use it
transparently; stub routines are provided to handle the C<->Fortran
interface without any modification of the user program.  C programs are
required to include the header file "plplot.h"; see the documentation for
more details.  Programmes written in many other languages can also use the
PLplot library.  The C++, Tcl, Python, Octave, and Yorick (this
front end is maintained independently, see yplot.sf.net) front ends to PLplot
are mature, the Java front end is almost there (most examples work), and
the Perl front end is still quite incomplete.

The PLplot package is distributable under the terms of the GNU Library General
Public License (LGPL) with some exceptions -- see the file "Copyright" for
full information.

Notes on the plplot configuration underlying this package for the
Mandrake build environment: 

(i) We configure with --enable-shared --enable-static --enable-drivers 
--enable-dyndrivers --enable-java --enable-python --enable-itcl 
--enable-gnome --disable-linuxvga --enable-builddoc --with-pthreads
--with-freetype --with-double --with-pkg-config
-with-xml-declaration=/usr/share/sgml/xml.dcl 
--with-sgml-catalogs=/etc/sgml/catalog
JAVA_HOME must be set properly to build the java package.

(ii) A large number of drivers are configured by default including tk, ps,
psc, png, and jpeg.

(iii) Base languages supported are c, c++, fortran 77, and tcl. All these
packages are standard on Mandrake.

(iv) Support for gd, gnome, java, octave, python, tcltk drivers and/or bindings
are available as separate subpackages.

The full configure output is as follows:

devices:         cgm dg300 png jpeg gnome hp7470 hp7580 lj_hpgl imp ljii ljiip mem ntk null pbm plmeta ps psc pstex xterm tek4010 tek4107 mskermit versaterm vlt conex tek4010f tek4107f tk tkwin xfig xwin

Available device drivers:
static
dynamic         cgm.la dg300.la gd.la gnome.la hpgl.la impress.la ljii.la ljiip.la mem.la ntk.la null.la pbm.la plmeta.la ps.la pstex.la tek.la tk.la tkwin.la xfig.la xwin.la

Compilation options:
with_debug:     no              with_opt:       yes
with_warn:      no              with_profile:   no

Library options:
enable_shared:  yes             enable_static:  yes
with_rpath:     yes             with_double:    yes

Optional libraries:
with_qhull:     yes             with_csa:       yes
with_freetype:  yes             with_pthreads:  yes


Language Bindings:
enable_tcl:     yes             enable_itcl:    yes
enable_cxx:     yes             enable_f77:     yes
enable_java:    yes             enable_python:  yes
enable_octave:  yes

%package -n %{lib_name}
Summary: Main libraries for PLplot
Group: System/Libraries
Provides: %{lib_name_orig} = %{version}-%{release}

%description -n %{lib_name}
This package contains the libraries needed to run programs dynamically
linked with PLplot, a scientific plotting library.

%package -n %{lib_name}-drivers
Summary: Main drivers for PLplot
Group: System/Libraries
Requires: %{lib_name} = %{version}
Provides: %{name}-drivers = %{version}-%{release}

%description -n %{lib_name}-drivers
This package contains the base output drivers for PLplot, 
a scientific plotting library.
Many different output device drivers are available, including postscript,
tektronix, HP laserjet, HP pen plotters, XFig, xterm, X-windows, and a
portable metafile format, among others.

%package -n %{lib_name_devel}
Summary: Headers and libraries for developing programs that will use PLplot
Group: Development/C
Requires: %{lib_name} = %{version}
Provides: %{lib_name_orig}-devel = %{version}-%{release}
Provides: %{name}-devel = %{version}-%{release}

%description -n %{lib_name_devel}
This package contains the headers, libraries, documentation and 
examples needed to develop applications using PLplot, a 
scientific plotting library.

%package -n %{lib_name_static_devel}
Summary: Static libraries for PLplot
Group: Development/C
Requires: %{lib_name_devel} = %{version}
Provides: %{lib_name_orig}-static-devel = %{version}-%{release}
Provides: %{name}-static-devel = %{version}-%{release}

%description -n %{lib_name_static_devel}
This package contains the static libraries for developing
programs which use plplot. Install this package if you need to statically
link your program or library.

%package -n %{lib_name}-driver-gd
Summary: GD graphics driver for PLplot
Group: System/Libraries
Requires: %{lib_name} = %{version}
Provides: %{name}-gd = %{version}-%{release}
Provides: %{lib_name}-gd = %{version}-%{release}

%description -n %{lib_name}-driver-gd
This package contains the GD graphics driver/plugin for PLplot,
which allows direct generation of PNG and JPEG files

%package -n %{lib_name}-driver-gnome
Summary: Gnome/Gtk based driver for PLplot
Group: System/Libraries
Requires: %{lib_name} = %{version}
Provides: %{name}-gnome = %{version}-%{release}
Provides: %{lib_name}-gnome = %{version}-%{release}

%description -n %{lib_name}-driver-gnome
This package contains the Gnome/Gtk based driver/plugin for PLplot.

%package -n %{lib_name_orig}-python
Summary: Python bindings for PLplot
Group: Development/Python
Requires: python >= 2.1 python-numeric >= 20.1 
Requires: %{lib_name} = %{version}
Provides: %{name}-python = %{version}-%{release}

%description -n %{lib_name_orig}-python
This package contains python wrappers for PLplot.

%package -n %{lib_name_orig}-octave
Summary: Octave bindings for PLplot
Group: Sciences/Mathematics
Requires: octave >= 2.0 %{lib_name} = %{version}
Provides: %{name}-octave = %{version}-%{release}

%description -n %{lib_name_orig}-octave
This package contains octave wrappers for PLplot.
Octave is a language for linear algebra and much more.
In order to use this package, issue the command "use_plplot" at the Octave
prompt.  This will have the effect of replacing the traditional
Gnuplot interface by a PLplot based one.

%package -n %{name}-tcltk
Summary: TclTk applications for PLplot
Group: Science/Mathematics
Requires: tcl >= 8.1 tk >= 8.1 
Requires: %{lib_name} = %{version} 
Requires: %{lib_name}-tcltk = %{version} 

%description -n %{name}-tcltk
This package contains Tcl/Tk applications pltcl, plserver for PLplot.
The pltcl program allows interactive plotting using the Tcl script 
language with plotting extensions.

%package -n %{lib_name}-tcltk
Summary: TclTk bindings and drivers for PLplot
Group: System/Libraries
Requires: tcl >= 8.1 tk >= 8.1 
Requires: %{lib_name} = %{version}
Provides: %{lib_name_orig}-tcltk = %{version}-%{release}

%description -n %{lib_name}-tcltk
This package contains Tcl/Tk drivers and wrappers for PLplot.

%package -n %{lib_name}-tcltk-devel
Summary: TclTk development libraries for PLplot
Group: System/Libraries
Requires: %{lib_name}-devel = %{version} %{lib_name}-tcltk = %{version}
Provides: %{lib_name_orig}-tcltk-devel = %{version}-%{release} 
Provides: %{name}-tcltk-devel = %{version}-%{release}

%description -n %{lib_name}-tcltk-devel
This package contains headers and libraries for making 
Tcl/Tk applications with PLplot.
This includes a Tk-based plotting widget, called a plframe.  This
widget is based on the Tk frame widget, and supports a variety of user
options such as zoom, scrolls, save to a variety of devices, print,
resize, page layout, orientation change, and palette manipulation.  Direct
plotting to the widget may be done by calls to the underlying library
using either C or Fortran bindings, and a large number of operations can
be interpreted directly (in Tcl) by the widget

%package -n %{lib_name}-tcltk-static-devel
Summary: Static TclTk libraries for PLplot
Group: Development/Other
Requires: %{lib_name}-tcltk-devel = %{version}
Provides: %{lib_name_orig}-tcltk-static-devel = %{version}-%{release} 
Provides: %{name}-tcltk-static-devel = %{version}-%{release}

%description -n %{lib_name}-tcltk-static-devel
This package contains the static libraries for developing
Tcl/Tk programs which use plplot. Install this package if you need to 
statically link your program or library.

%package -n %{lib_name}-java
Summary: Java bindings for PLplot
Group: Development/Java
Requires: java 
Requires: %{lib_name} = %{version}
Provides: %{name}-java = %{version}-%{release}

%description -n %{lib_name}-java
This package contains Java wrappers for PLplot.

%prep

%setup -q
# Only if --enable-linuxvga
#%%patch0 -p1

%build
# make sure JAVA_HOME environment var is set if you want java bindings.
%configure --enable-shared --enable-static --enable-drivers --enable-dyndrivers --enable-java --enable-python --enable-itcl --enable-gnome --disable-linuxvga --enable-builddoc --with-pthreads --with-pkg-config -with-xml-declaration=/usr/share/sgml/xml.dcl --with-sgml-catalogs=/etc/sgml/catalog

# Don't do a multithreaded make here
make

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%makeinstall_std
# This macro is the same as:
#make DESTDIR=$RPM_BUILD_ROOT install

# Some doc fixups since %doc removes the existing doc install area!
mkdir -p doc/html
cp -a doc/docbook/src/*html doc/html
cp -a %{buildroot}/%{_datadir}/doc/plplot/README.pkg-config .
cp -a %{buildroot}/%{_datadir}/doc/plplot/*.csa .
cp -a %{buildroot}/%{_datadir}/doc/plplot/*.nn .
rm -rf %{buildroot}/%{_datadir}/doc/plplot

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%post -n %{lib_name}
/sbin/ldconfig

%postun -n %{lib_name}
/sbin/ldconfig

%post -n %{lib_name}-tcltk
/sbin/ldconfig

%postun -n %{lib_name}-tcltk
/sbin/ldconfig

%post -n %{lib_name_devel}
/sbin/install-info --entry="* PLplot: (plplotdoc).  PLplot plotting suite." /usr/share/info/plplotdoc.info.bz2 /usr/share/info/dir

%preun -n %{lib_name_devel}
%_remove_install_info plplotdoc.info
#if [ $1 = 0 ]; then
#   /sbin/install-info --delete /usr/share/info/plplotdoc.info.bz2 /usr/share/info/dir
#fi


%files
%defattr(-,root,root)
# base docs
%doc AUTHORS ChangeLog COPYING.LIB Copyright FAQ NEWS PROBLEMS README SERVICE ToDo TODO.AM-LT README.1st.csa README.1st.nn README.csa drivers/README.drivers README.nn
%{_mandir}/man1/plm2gif.1.bz2 
%{_mandir}/man1/plpr.1.bz2 
%{_mandir}/man1/plrender.1.bz2
%{_mandir}/man1/pltek.1.bz2 
%{_mandir}/man1/pstex2eps.1.bz2

# executables
%{_bindir}/plm2gif
%{_bindir}/plpr
%{_bindir}/plrender
%{_bindir}/pltek
%{_bindir}/pstex2eps

# fonts and maps 
%dir %{_datadir}/plplot%{version}
%{_datadir}/plplot%{version}/*.fnt
%{_datadir}/plplot%{version}/*.map

%files -n %{lib_name}
%defattr(-,root,root)
# libraries
%{_libdir}/*plplotf77*.so.*
%{_libdir}/*plplotcxx*.so.*
%{_libdir}/*plplotd.so.*
%{_libdir}/*nn*.so.*
%{_libdir}/*csa*.so.*

# driver area
%dir %{_libdir}/plplot%{version}
%dir %{_libdir}/plplot%{version}/driversd

# Base drivers package
%files -n %{lib_name}-drivers
%defattr(-,root,root)
# drivers
%{_libdir}/plplot%{version}/driversd/cgm*
%{_libdir}/plplot%{version}/driversd/dg300*
%{_libdir}/plplot%{version}/driversd/hpgl*
%{_libdir}/plplot%{version}/driversd/impress*
#%%{_libdir}/plplot%{version}/driversd/linuxvga*
%{_libdir}/plplot%{version}/driversd/ljii*
%{_libdir}/plplot%{version}/driversd/mem*
%{_libdir}/plplot%{version}/driversd/null*
%{_libdir}/plplot%{version}/driversd/pbm*
%{_libdir}/plplot%{version}/driversd/plmeta*
%{_libdir}/plplot%{version}/driversd/ps.*
%{_libdir}/plplot%{version}/driversd/pstex*
%{_libdir}/plplot%{version}/driversd/tek*
%{_libdir}/plplot%{version}/driversd/xfig*
%{_libdir}/plplot%{version}/driversd/xwin*

# Devel package
%files -n %{lib_name_devel}
%defattr(-,root,root)
# More extensive documentation
%{_mandir}/man3/*.3plplot.bz2 
%doc doc/docbook/src/plplot-%{version}.dvi doc/docbook/src/plplot-%{version}.pdf doc/docbook/src/plplot-%{version}.ps.gz doc/html README.pkg-config
# info files
%{_datadir}/info/plplotdoc.info*.bz2
%{_bindir}/plplot-config*
%{_mandir}/man1/plplot_libtool.1.bz2
%{_bindir}/plplot_libtool
# headers
%dir %{_includedir}/plplot
%{_includedir}/plplot/disptab.h
%{_includedir}/plplot/drivers.h
%{_includedir}/plplot/pdf.h
%{_includedir}/plplot/plConfig.h
%{_includedir}/plplot/pldebug.h
%{_includedir}/plplot/plDevs.h
%{_includedir}/plplot/plevent.h
%{_includedir}/plplot/plplot.h
%{_includedir}/plplot/plplotP.h
%{_includedir}/plplot/plstream.h
%{_includedir}/plplot/plstrm.h
%{_includedir}/plplot/plxwd.h
# libs
%{_libdir}/*plplotf77*.so
%{_libdir}/*plplotcxx*.so
%{_libdir}/*plplotd.so
%{_libdir}/*nn*.so
%{_libdir}/*csa*.so
%{_libdir}/*plplotf77*.la
%{_libdir}/*plplotcxx*.la
%{_libdir}/*plplotd.la
%{_libdir}/*nn*.la
%{_libdir}/*csa*.la

%{_libdir}/pkgconfig
%dir %{_datadir}/plplot%{version}/examples
%{_datadir}/plplot%{version}/examples/Makefile
%{_datadir}/plplot%{version}/examples/c
%{_datadir}/plplot%{version}/examples/c++
%{_datadir}/plplot%{version}/examples/f77
%{_datadir}/plplot%{version}/examples/perl
%{_datadir}/plplot%{version}/examples/plplot-test.sh
%{_datadir}/plplot%{version}/examples/test_c.sh
%{_datadir}/plplot%{version}/examples/test_cxx.sh
%{_datadir}/plplot%{version}/examples/test_f77.sh

%files -n %{lib_name_static_devel}
%defattr(-,root,root)
%{_libdir}/*plplotf77*.a
%{_libdir}/*plplotcxx*.a
%{_libdir}/*plplotd.a
%{_libdir}/*nn*.a
%{_libdir}/*csa*.a

# for gd
%files -n %{lib_name}-driver-gd
%defattr(-,root,root)
%{_libdir}/plplot%{version}/driversd/gd*

# for gnome
%files -n %{lib_name}-driver-gnome
%defattr(-,root,root)
%{_libdir}/plplot%{version}/driversd/gnome*

# for python
%files -n %{lib_name_orig}-python
%defattr(-,root,root)
%{_libdir}/python*
%{_datadir}/plplot%{version}/examples/test_python.sh
%{_datadir}/plplot%{version}/examples/python

# for octave
%files -n %{lib_name_orig}-octave
%defattr(-,root,root)
%{_libdir}/octave
%{_datadir}/plplot%{version}/examples/test_octave.sh
%{_datadir}/plplot%{version}/examples/octave
%{_datadir}/octave
%{_datadir}/plplot_octave
%doc bindings/octave/BUGS bindings/octave/INSTALL bindings/octave/README bindings/octave/USAGE bindings/octave/ToDo bindings/octave/FGA

# for java
%files -n %{lib_name}-java
%defattr(-,root,root)
%{_libdir}/java
%{_datadir}/plplot%{version}/examples/test_java.sh

# for tcltk apps
%files -n %{name}-tcltk
%defattr(-,root,root)
%{_mandir}/man1/plserver.1.bz2
%{_mandir}/man1/pltcl.1.bz2
%{_bindir}/plserver
%{_bindir}/pltcl

# for tcltk
%files -n %{lib_name}-tcltk
%defattr(-,root,root)
%{_libdir}/*tcl*.so.*
%{_datadir}/plplot%{version}/pkgIndex.tcl
%{_datadir}/plplot%{version}/tcl
%{_libdir}/plplot%{version}/driversd/*tk*

# for tcltk-devel
%files -n %{lib_name}-tcltk-devel
%defattr(-,root,root)
%{_libdir}/*tcl*.so
%{_libdir}/*tcl*.la
%{_includedir}/plplot/pltcl.h
%{_includedir}/plplot/pltk.h
%{_includedir}/plplot/tclMatrix.h
%{_datadir}/plplot%{version}/examples/tk
%{_datadir}/plplot%{version}/examples/test_tcl.sh
%{_datadir}/plplot%{version}/examples/tcl

# for tcltk-static-devel
%files -n %{lib_name}-tcltk-static-devel
%defattr(-,root,root)
%{_libdir}/*tcl*.a

%changelog
* Tue Jan 27 2004 Brian D. Wright <bdwright@phy.ucsf.edu> 5.2.1.rc2.5.3.0-2mdk
- Added explicit directories for proper cleanup on uninstall.
- Added missing defattrs.

* Tue Jan 27 2004 Brian D. Wright <bdwright@phy.ucsf.edu> 5.2.1.rc2.5.3.0-1mdk
- Update for new release candidate.

* Tue Jan 20 2004 Brian D. Wright <bdwright@phy.ucsf.edu> 5.2.1.rc1.5.3.0-1mdk
- Update for new release candidate.
- Set --with-pkg-config option in configure.
- Moved data and examples to datadir install locations.


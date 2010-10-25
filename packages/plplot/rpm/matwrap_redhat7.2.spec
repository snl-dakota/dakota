# Create rpm binary and source packages with ordinary user account using
# safe temporary install location with eventual root ownership of files that 
# are actually installed in the final system location by the binary rpm.

BuildRoot: /tmp/software/redhat_install_area/matwrap
# N.B. note use of $RPM_BUILD_ROOT (which points to this location) in the 
# %prep and %install areas, and explicit removal of this location in 
# %install area before actual install to this location occurs.

%define version 0.57
%define rpmversion 1rh7.2

Summary: matwrap is a wrapper generator for matrix languages
Packager: Alan W. Irwin <irwin@beluga.phys.uvic.ca>
Name: matwrap
Version: %{version}
Release: %{rpmversion}
Source0: http://lnc.usc.edu/~holt/matwrap/matwrap-%{version}.tar.gz
URL: http://lnc.usc.edu/~holt/matwrap/
Copyright: Perl Artistic License
Group: Applications/Math
requires: perl >= 5.004

%description
Matwrap is a tool for interfacing C++ code into matrix-oriented scripting
languages such as Matlab 5, Octave, and tela. It generates all the code to
convert from the scripting language's internal types into the types that
your C++ code understands (e.g., double, char *, float *, struct abc *). You
do not need to understand any of the API details of the language to use your
C++ code; just give matwrap a .h file describing your functions.

 Brief list of features: 
 
 + Functions are automatically vectorized. 
 + Arguments containing dimensions of other vector and matrix arguments can be
   computed automatically and need not be specified. 
 + Pointers to structures and classes are supported. Public member functions of
   classes may be called, and public data members may be evaluated or set.
   Inheritance is supported. 

%prep
%setup -n matwrap
%build
%install
# remove safe install area before installing to it, but be explicit 
# because $RPM_BUILD_ROOT (which is set to this area within this script)
# can be overridden in error from outside
rm -rf /tmp/software/redhat_install_area/matwrap
# Must create top level prefix directory because of bug in install.pl
mkdir -p $RPM_BUILD_ROOT/usr
# The PREFIX value makes this package relocatable.
make install PREFIX=$RPM_BUILD_ROOT/usr
# Install puts things in the wrong place (or not at all) so move things
# around to compensate.
mkdir -p $RPM_BUILD_ROOT/usr/share/doc/matwrap
mv README TODO $RPM_BUILD_ROOT/usr/share/doc/matwrap
mv $RPM_BUILD_ROOT/usr/share/matwrap/Examples $RPM_BUILD_ROOT/usr/share/doc/matwrap/examples

%files
# doc
%attr(-, root, root) %doc /usr/share/doc/matwrap
# shared language modules
%attr(-, root, root) %doc /usr/share/matwrap
# executables
%attr(-, root, root) /usr/bin/matwrap

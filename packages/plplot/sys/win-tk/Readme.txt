If running on any windows platform, and you have VC++ installed, you
should be able to do 'nmake -f makefile.vc' to compile plplot into a
shared library (.dll) for use by Tk.  You will probably need to edit a
couple of paths in makefile.vc for the moment.

You can then use 'nmake -f makefile.vc install' to install (and 'clean' to
clean up).  Also 'nmake -f makefile.vc install-examples' is useful.

After installation, starting your Tk shell and typing 'package require
Plplotter' should load the .dll you just installed.

Note: the various .c, .h, *.tcl files in this directory are files
which would normally be generated using various
auto_conf/configure/perl scripts in the standard Unix distribution.
Since these cannot (or cannot easily) be generated on Windows, they
are included here.

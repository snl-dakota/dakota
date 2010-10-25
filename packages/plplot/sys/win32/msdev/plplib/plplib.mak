# Microsoft Developer Studio Generated NMAKE File, Based on plplib.dsp
!IF "$(CFG)" == ""
CFG=plplib - Win32 Release
!MESSAGE No configuration specified. Defaulting to plplib - Win32 Debug.
!ENDIF

!IF "$(CFG)" != "plplib - Win32 Release" && "$(CFG)" != "plplib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE
!MESSAGE NMAKE /f "plplib.mak" CFG="plplib - Win32 Debug"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "plplib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "plplib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

CPP=cl.exe
F90=df.exe
RSC=rc.exe

!IF  "$(CFG)" == "plplib - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
TMPDIR=..\..\..\..\tmp
LIBDIR=..\..\..\..\lib
BUILD_DIR="\"tmp\""

ALL : $(LIBDIR)\plplot.lib


CLEAN :
	-@erase "$(INTDIR)\null.obj"
	-@erase "$(INTDIR)\pbm.obj"
	-@erase "$(INTDIR)\pdfutils.obj"
	-@erase "$(INTDIR)\plargs.obj"
	-@erase "$(INTDIR)\plbox.obj"
	-@erase "$(INTDIR)\plbuf.obj"
	-@erase "$(INTDIR)\plcont.obj"
	-@erase "$(INTDIR)\plcore.obj"
	-@erase "$(INTDIR)\plctrl.obj"
	-@erase "$(INTDIR)\plcvt.obj"
	-@erase "$(INTDIR)\pldtik.obj"
	-@erase "$(INTDIR)\plfill.obj"
	-@erase "$(INTDIR)\plhist.obj"
	-@erase "$(INTDIR)\plimage.obj"
	-@erase "$(INTDIR)\plline.obj"
	-@erase "$(INTDIR)\plmap.obj"
	-@erase "$(INTDIR)\plmeta.obj"
	-@erase "$(INTDIR)\plot3d.obj"
	-@erase "$(INTDIR)\plpage.obj"
	-@erase "$(INTDIR)\plrender.obj"
	-@erase "$(INTDIR)\plsdef.obj"
	-@erase "$(INTDIR)\plshade.obj"
	-@erase "$(INTDIR)\plstripc.obj"
	-@erase "$(INTDIR)\plsym.obj"
	-@erase "$(INTDIR)\pltick.obj"
	-@erase "$(INTDIR)\plvpor.obj"
	-@erase "$(INTDIR)\plwind.obj"
	-@erase "$(INTDIR)\ps.obj"
	-@erase "$(INTDIR)\xfig.obj"
	-@erase "$(INTDIR)\mem.obj"
	-@erase "$(INTDIR)\hpgl.obj"
	-@erase "$(INTDIR)\pstex.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\win3.obj"
	-@erase ".\plplot.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

F90_PROJ=/compile_only /include:"$(INTDIR)\\" /nologo /warn:nofileopt /module:"Release/" /object:"Release/"
F90_OBJS=.\Release/
CPP_PROJ=/nologo /Ox /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "BUILD_DIR=$(BUILD_DIR)" /D "HAVE_FREETYPE=0" /I..\..\..\..\..\freetype\freetype-2.1.9\include /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\plplib.bsc"
BSC32_SBRS= \
	
LIB32=link.exe -lib
#LIB32_FLAGS=/nologo /out:"plplot.lib" "$(TMPDIR)\freetype.lib"
LIB32_FLAGS=/nologo /out:"plplot.lib"
LIB32_OBJS= \
	"$(INTDIR)\plcore.obj" \
	"$(INTDIR)\win3.obj" \
	"$(INTDIR)\plargs.obj" \
	"$(INTDIR)\pbm.obj" \
	"$(INTDIR)\null.obj" \
	"$(INTDIR)\ps.obj" \
	"$(INTDIR)\hpgl.obj" \
	"$(INTDIR)\mem.obj" \
	"$(INTDIR)\xfig.obj" \
	"$(INTDIR)\pstex.obj" \
	"$(INTDIR)\plbuf.obj" \
	"$(INTDIR)\plcont.obj" \
	"$(INTDIR)\plctrl.obj" \
	"$(INTDIR)\plcvt.obj" \
	"$(INTDIR)\plfill.obj" \
	"$(INTDIR)\plhist.obj" \
	"$(INTDIR)\plimage.obj" \
	"$(INTDIR)\plline.obj" \
	"$(INTDIR)\plmap.obj" \
	"$(INTDIR)\plmeta.obj" \
	"$(INTDIR)\plot3d.obj" \
	"$(INTDIR)\plpage.obj" \
	"$(INTDIR)\plrender.obj" \
	"$(INTDIR)\plsdef.obj" \
	"$(INTDIR)\plshade.obj" \
	"$(INTDIR)\plstripc.obj" \
	"$(INTDIR)\plsym.obj" \
	"$(INTDIR)\plbox.obj" \
	"$(INTDIR)\plvpor.obj" \
	"$(INTDIR)\pltick.obj" \
	"$(INTDIR)\plwind.obj" \
	"$(INTDIR)\pdfutils.obj" \
	"$(INTDIR)\plvect.obj" \
	"$(INTDIR)\pldtik.obj"

$(LIBDIR)\plplot.lib : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<
    copy .\plplot.lib $(LIBDIR)\plplot.lib

!ELSEIF  "$(CFG)" == "plplib - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\plplib.lib"


CLEAN :
	-@erase "$(INTDIR)\null.obj"
	-@erase "$(INTDIR)\pbm.obj"
	-@erase "$(INTDIR)\pdfutils.obj"
	-@erase "$(INTDIR)\plargs.obj"
	-@erase "$(INTDIR)\plbox.obj"
	-@erase "$(INTDIR)\plbuf.obj"
	-@erase "$(INTDIR)\plcont.obj"
	-@erase "$(INTDIR)\plcore.obj"
	-@erase "$(INTDIR)\plctrl.obj"
	-@erase "$(INTDIR)\plcvt.obj"
	-@erase "$(INTDIR)\pldtik.obj"
	-@erase "$(INTDIR)\plfill.obj"
	-@erase "$(INTDIR)\plhist.obj"
	-@erase "$(INTDIR)\plimage.obj"
	-@erase "$(INTDIR)\plline.obj"
	-@erase "$(INTDIR)\plmap.obj"
	-@erase "$(INTDIR)\plmeta.obj"
	-@erase "$(INTDIR)\plot3d.obj"
	-@erase "$(INTDIR)\plpage.obj"
	-@erase "$(INTDIR)\plrender.obj"
	-@erase "$(INTDIR)\plsdef.obj"
	-@erase "$(INTDIR)\plshade.obj"
	-@erase "$(INTDIR)\plstripc.obj"
	-@erase "$(INTDIR)\plsym.obj"
	-@erase "$(INTDIR)\pltick.obj"
	-@erase "$(INTDIR)\plvpor.obj"
	-@erase "$(INTDIR)\plvect.obj"
	-@erase "$(INTDIR)\plwind.obj"
	-@erase "$(INTDIR)\ps.obj"
	-@erase "$(INTDIR)\hpgl.obj"
	-@erase "$(INTDIR)\mem.obj"
	-@erase "$(INTDIR)\xfig.obj"
	-@erase "$(INTDIR)\pstex.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(INTDIR)\win3.obj"
	-@erase "$(OUTDIR)\plplib.lib"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

F90_PROJ=/check:bounds /compile_only /debug:full /include:"$(INTDIR)\\" /nologo /traceback /warn:argument_checking /warn:nofileopt /module:"Debug/" /object:"Debug/" /pdbfile:"Debug/DF60.PDB"
F90_OBJS=.\Debug/
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "HAVE_FREETYPE" /Fp"$(INTDIR)\plplib.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ  /c
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\plplib.bsc"
BSC32_SBRS= \
	
LIB32=link.exe -lib
LIB32_FLAGS=/nologo /out:"$(OUTDIR)\plplib.lib"
LIB32_OBJS= \
	"$(INTDIR)\plcore.obj" \
	"$(INTDIR)\win3.obj" \
	"$(INTDIR)\plargs.obj" \
	"$(INTDIR)\pbm.obj" \
	"$(INTDIR)\null.obj" \
	"$(INTDIR)\ps.obj" \
	"$(INTDIR)\hpgl.obj" \
	"$(INTDIR)\mem.obj" \
	"$(INTDIR)\xfig.obj" \
	"$(INTDIR)\pstex.obj" \
	"$(INTDIR)\plbuf.obj" \
	"$(INTDIR)\plcont.obj" \
	"$(INTDIR)\plctrl.obj" \
	"$(INTDIR)\plcvt.obj" \
	"$(INTDIR)\plfill.obj" \
	"$(INTDIR)\plhist.obj" \
	"$(INTDIR)\plimage.obj" \
	"$(INTDIR)\plline.obj" \
	"$(INTDIR)\plmap.obj" \
	"$(INTDIR)\plmeta.obj" \
	"$(INTDIR)\plot3d.obj" \
	"$(INTDIR)\plpage.obj" \
	"$(INTDIR)\plrender.obj" \
	"$(INTDIR)\plsdef.obj" \
	"$(INTDIR)\plshade.obj" \
	"$(INTDIR)\plstripc.obj" \
	"$(INTDIR)\plsym.obj" \
	"$(INTDIR)\plbox.obj" \
	"$(INTDIR)\plvpor.obj" \
	"$(INTDIR)\plvect.obj" \
	"$(INTDIR)\pltick.obj" \
	"$(INTDIR)\plwind.obj" \
	"$(INTDIR)\pdfutils.obj" \
	"$(INTDIR)\pldtik.obj"

"$(OUTDIR)\plplib.lib" : "$(OUTDIR)" $(DEF_FILE) $(LIB32_OBJS)
    $(LIB32) @<<
  $(LIB32_FLAGS) $(DEF_FLAGS) $(LIB32_OBJS)
<<

!ENDIF

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $<
<<

.SUFFIXES: .fpp

.for{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<

.f{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<

.f90{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<

.fpp{$(F90_OBJS)}.obj:
   $(F90) $(F90_PROJ) $<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("plplib.dep")
!INCLUDE "plplib.dep"
!ELSE
!MESSAGE Warning: cannot find "plplib.dep"
!ENDIF
!ENDIF


!IF "$(CFG)" == "plplib - Win32 Release" || "$(CFG)" == "plplib - Win32 Debug"
SOURCE=$(TMPDIR)\null.c

"$(INTDIR)\null.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\pbm.c

"$(INTDIR)\pbm.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\pdfutils.c

"$(INTDIR)\pdfutils.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plargs.c

"$(INTDIR)\plargs.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plbox.c

"$(INTDIR)\plbox.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plbuf.c

"$(INTDIR)\plbuf.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plcont.c

"$(INTDIR)\plcont.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plcore.c

"$(INTDIR)\plcore.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plctrl.c

"$(INTDIR)\plctrl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plcvt.c

"$(INTDIR)\plcvt.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\pldtik.c

"$(INTDIR)\pldtik.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plfill.c

"$(INTDIR)\plfill.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plhist.c

"$(INTDIR)\plhist.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plimage.c

"$(INTDIR)\plimage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plline.c

"$(INTDIR)\plline.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plmap.c

"$(INTDIR)\plmap.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plmeta.c

"$(INTDIR)\plmeta.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plot3d.c

"$(INTDIR)\plot3d.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plpage.c

"$(INTDIR)\plpage.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plrender.c

"$(INTDIR)\plrender.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plsdef.c

"$(INTDIR)\plsdef.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plshade.c

"$(INTDIR)\plshade.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plstripc.c

"$(INTDIR)\plstripc.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plsym.c

"$(INTDIR)\plsym.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\pltick.c

"$(INTDIR)\pltick.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plvect.c

"$(INTDIR)\plvect.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plvpor.c

"$(INTDIR)\plvpor.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\plwind.c

"$(INTDIR)\plwind.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\ps.c

"$(INTDIR)\ps.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\gd.c

"$(INTDIR)\gd.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE) /I..\..\..\..\gd\include

#
# Does not work correctly yet
#
SOURCE=$(TMPDIR)\plfreetype.c

"$(INTDIR)\plfreetype.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE) /I..\..\..\..\gd\include /D "HAVE_FREETYPE"


SOURCE=$(TMPDIR)\pstex.c

"$(INTDIR)\pstex.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\hpgl.c

"$(INTDIR)\hpgl.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\mem.c

"$(INTDIR)\mem.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\xfig.c

"$(INTDIR)\xfig.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=$(TMPDIR)\win3.cpp

"$(INTDIR)\win3.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)



!ENDIF


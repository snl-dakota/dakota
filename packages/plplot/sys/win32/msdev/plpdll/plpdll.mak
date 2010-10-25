# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

!IF "$(CFG)" == ""
CFG=plpdll - Win32 Release
!MESSAGE No configuration specified.  Defaulting to plpdll - Win32 Release.
!ENDIF

!IF "$(CFG)" != "plpdll - Win32 Release" && "$(CFG)" != "plpdll - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE
!MESSAGE NMAKE /f "plpdll.mak" CFG="plpdll - Win32 Release"
!MESSAGE
!MESSAGE Possible choices for configuration are:
!MESSAGE
!MESSAGE "plpdll - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "plpdll - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE
!ERROR An invalid configuration is specified.
!ENDIF

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF
################################################################################
# Begin Project
# PROP Target_Last_Scanned "plpdll - Win32 Debug"
MTL=mktyplib.exe
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "plpdll - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
OUTDIR=.\Release
INTDIR=.\Release
BUILD_DIR=\"tmp\"

ALL : "$(OUTDIR)\plplotd.dll"

CLEAN :
	-@erase ".\plplotd.dll"
	-@erase ".\Release\plconsole.obj"
	-@erase ".\Release\plctrl.obj"
	-@erase ".\Release\plot3d.obj"
	-@erase ".\Release\plbox.obj"
	-@erase ".\Release\ps.obj"
	-@erase ".\Release\pltick.obj"
	-@erase ".\Release\pdfutils.obj"
	-@erase ".\Release\plfill.obj"
	-@erase ".\Release\plstub.obj"
	-@erase ".\Release\plpage.obj"
	-@erase ".\Release\plmeta.obj"
	-@erase ".\Release\xfig.obj"
	-@erase ".\Release\plwind.obj"
	-@erase ".\Release\plcvt.obj"
	-@erase ".\Release\plbuf.obj"
	-@erase ".\Release\plsdef.obj"
	-@erase ".\Release\plsym.obj"
	-@erase ".\Release\win3.obj"
	-@erase ".\Release\plmap.obj"
	-@erase ".\Release\plargs.obj"
	-@erase ".\Release\plline.obj"
	-@erase ".\Release\plhist.obj"
	-@erase ".\Release\plvect.obj"
	-@erase ".\Release\plvpor.obj"
	-@erase ".\Release\plcont.obj"
	-@erase ".\Release\plimage.obj"
	-@erase ".\Release\plshade.obj"
	-@erase ".\Release\pldtik.obj"
	-@erase ".\Release\plcore.obj"
	-@erase ".\Release\hpgl.obj"
	-@erase ".\Release\null.obj"
	-@erase ".\Release\plplot.res"
	-@erase ".\Release\plplotd.lib"
	-@erase ".\Release\plplotd.exp"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MT /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_WINDOWS"\
/D "BUILD_DIR=$(BUILD_DIR)" /Fp"$(INTDIR)/plpdll.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE MTL /nologo /D "NDEBUG" /win32
# ADD MTL /nologo /D "NDEBUG" /win32
MTL_PROJ=/nologo /D "NDEBUG" /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/plplot.res" /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/plpdll.bsc"
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /machine:I386 /out:"plplotd.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:no\
 /pdb:"$(OUTDIR)/plplotd.pdb" /machine:I386 /def:"..\..\..\..\tmp\plplotd.def"\
 /out:"plplotd.dll" /implib:"$(OUTDIR)/plplotd.lib"
DEF_FILE= \
	"..\..\..\..\tmp\plplotd.def"
LINK32_OBJS= \
	".\Release\plconsole.obj" \
	".\Release\plctrl.obj" \
	".\Release\plot3d.obj" \
	".\Release\plbox.obj" \
	".\Release\ps.obj" \
	".\Release\pltick.obj" \
	".\Release\pdfutils.obj" \
	".\Release\plfill.obj" \
	".\Release\plstub.obj" \
	".\Release\plpage.obj" \
	".\Release\plmeta.obj" \
	".\Release\xfig.obj" \
	".\Release\plwind.obj" \
	".\Release\plcvt.obj" \
	".\Release\plbuf.obj" \
	".\Release\plsdef.obj" \
	".\Release\plsym.obj" \
	".\Release\win3.obj" \
	".\Release\plmap.obj" \
	".\Release\plargs.obj" \
	".\Release\plline.obj" \
	".\Release\plhist.obj" \
	".\Release\plvect.obj" \
	".\Release\plvpor.obj" \
	".\Release\plcont.obj" \
	".\Release\plimage.obj" \
	".\Release\plshade.obj" \
	".\Release\pldtik.obj" \
	".\Release\plcore.obj" \
	".\Release\hpgl.obj" \
	".\Release\null.obj" \
	".\Release\plplot.res"

"$(OUTDIR)\plplotd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "plpdll - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
OUTDIR=.\Debug
INTDIR=.\Debug
BUILD_DIR=\"tmp\"

ALL : "$(OUTDIR)\plplotd.dll"

CLEAN :
	-@erase ".\Debug\vc40.pdb"
	-@erase ".\Debug\vc40.idb"
	-@erase ".\plplotd.dll"
	-@erase ".\Debug\pdfutils.obj"
	-@erase ".\Debug\plot3d.obj"
	-@erase ".\Debug\plbox.obj"
	-@erase ".\Debug\ps.obj"
	-@erase ".\Debug\plstub.obj"
	-@erase ".\Debug\plmeta.obj"
	-@erase ".\Debug\plconsole.obj"
	-@erase ".\Debug\plbuf.obj"
	-@erase ".\Debug\plsdef.obj"
	-@erase ".\Debug\plmap.obj"
	-@erase ".\Debug\plargs.obj"
	-@erase ".\Debug\plctrl.obj"
	-@erase ".\Debug\pltick.obj"
	-@erase ".\Debug\plfill.obj"
	-@erase ".\Debug\plimage.obj"
	-@erase ".\Debug\plshade.obj"
	-@erase ".\Debug\plpage.obj"
	-@erase ".\Debug\plcont.obj"
	-@erase ".\Debug\xfig.obj"
	-@erase ".\Debug\plwind.obj"
	-@erase ".\Debug\plcvt.obj"
	-@erase ".\Debug\hpgl.obj"
	-@erase ".\Debug\pldtik.obj"
	-@erase ".\Debug\plcore.obj"
	-@erase ".\Debug\plsym.obj"
	-@erase ".\Debug\null.obj"
	-@erase ".\Debug\win3.obj"
	-@erase ".\Debug\plline.obj"
	-@erase ".\Debug\plhist.obj"
	-@erase ".\Debug\plvect.obj"
	-@erase ".\Debug\plvpor.obj"
	-@erase ".\Debug\plplot.res"
	-@erase ".\plplotd.ilk"
	-@erase ".\Debug\plplotd.lib"
	-@erase ".\Debug\plplotd.exp"
	-@erase ".\Debug\plplotd.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /YX /c
CPP_PROJ=/nologo /MTd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_WINDOWS"\
 /Fp"$(INTDIR)/plpdll.pch" /D "BUILD_DIR=$(BUILD_DIR)" /YX /Fo"$(INTDIR)/"
/Fd"$(INTDIR)/"
/c CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE MTL /nologo /D "_DEBUG" /win32
# ADD MTL /nologo /D "_DEBUG" /win32
MTL_PROJ=/nologo /D "_DEBUG" /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
RSC_PROJ=/l 0x409 /fo"$(INTDIR)/plplot.res" /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/plpdll.bsc"
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"plplotd.dll"
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:windows /dll /incremental:yes\
 /pdb:"$(OUTDIR)/plplotd.pdb" /debug /machine:I386\
 /def:"..\..\..\..\tmp\plplotd.def" /out:"plplotd.dll"\
 /implib:"$(OUTDIR)/plplotd.lib"
DEF_FILE= \
	"..\..\..\..\tmp\plplotd.def"
LINK32_OBJS= \
	".\Debug\pdfutils.obj" \
	".\Debug\plot3d.obj" \
	".\Debug\plbox.obj" \
	".\Debug\ps.obj" \
	".\Debug\plstub.obj" \
	".\Debug\plmeta.obj" \
	".\Debug\plconsole.obj" \
	".\Debug\plbuf.obj" \
	".\Debug\plsdef.obj" \
	".\Debug\plmap.obj" \
	".\Debug\plargs.obj" \
	".\Debug\plctrl.obj" \
	".\Debug\pltick.obj" \
	".\Debug\plfill.obj" \
	".\Debug\plimage.obj" \
	".\Debug\plshade.obj" \
	".\Debug\plpage.obj" \
	".\Debug\plcont.obj" \
	".\Debug\xfig.obj" \
	".\Debug\plwind.obj" \
	".\Debug\plcvt.obj" \
	".\Debug\hpgl.obj" \
	".\Debug\pldtik.obj" \
	".\Debug\plcore.obj" \
	".\Debug\plsym.obj" \
	".\Debug\null.obj" \
	".\Debug\win3.obj" \
	".\Debug\plline.obj" \
	".\Debug\plhist.obj" \
	".\Debug\plvect.obj" \
	".\Debug\plvpor.obj" \
	".\Debug\plplot.res"

"$(OUTDIR)\plplotd.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF

.c{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<

.cpp{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<

.cxx{$(CPP_OBJS)}.obj:
   $(CPP) $(CPP_PROJ) $<

.c{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<

.cpp{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<

.cxx{$(CPP_SBRS)}.sbr:
   $(CPP) $(CPP_PROJ) $<

################################################################################
# Begin Target

# Name "plpdll - Win32 Release"
# Name "plpdll - Win32 Debug"

!IF  "$(CFG)" == "plpdll - Win32 Release"

!ELSEIF  "$(CFG)" == "plpdll - Win32 Debug"

!ENDIF

################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\xfig.c
DEP_CPP_XFIG_=\
	".\..\..\..\..\tmp\plDevs.h"\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

!IF  "$(CFG)" == "plpdll - Win32 Release"


"$(INTDIR)\xfig.obj" : $(SOURCE) $(DEP_CPP_XFIG_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "plpdll - Win32 Debug"


"$(INTDIR)\xfig.obj" : $(SOURCE) $(DEP_CPP_XFIG_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\win3.cpp

!IF  "$(CFG)" == "plpdll - Win32 Release"

DEP_CPP_WIN3_=\
	".\..\..\..\..\tmp\plDevs.h"\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\win3.obj" : $(SOURCE) $(DEP_CPP_WIN3_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "plpdll - Win32 Debug"

DEP_CPP_WIN3_=\
	".\..\..\..\..\tmp\plDevs.h"\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	
NODEP_CPP_WIN3_=\
	".\..\..\..\..\tmp\CreatePalette"\
	".\..\..\..\..\tmp\dev"\
	".\..\..\..\..\tmp\LocalAlloc"\
	

"$(INTDIR)\win3.obj" : $(SOURCE) $(DEP_CPP_WIN3_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\ps.c
DEP_CPP_PS_CP=\
	".\..\..\..\..\tmp\plDevs.h"\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\ps.obj" : $(SOURCE) $(DEP_CPP_PS_CP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plwind.c
DEP_CPP_PLWIN=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plwind.obj" : $(SOURCE) $(DEP_CPP_PLWIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plvpor.c
DEP_CPP_PLVPO=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plvpor.obj" : $(SOURCE) $(DEP_CPP_PLVPO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plvect.c
DEP_CPP_PLVEO=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plvect.obj" : $(SOURCE) $(DEP_CPP_PLVPO) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\pltick.c
DEP_CPP_PLTIC=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\pltick.obj" : $(SOURCE) $(DEP_CPP_PLTIC) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plsym.c
DEP_CPP_PLSYM=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plsym.obj" : $(SOURCE) $(DEP_CPP_PLSYM) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plstub.cpp
DEP_CPP_PLSTU=\
	".\..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\plstub.obj" : $(SOURCE) $(DEP_CPP_PLSTU) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plimage.c
DEP_CPP_PLIMA=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plimage.obj" : $(SOURCE) $(DEP_CPP_PLIMA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plshade.c
DEP_CPP_PLSHA=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plshade.obj" : $(SOURCE) $(DEP_CPP_PLSHA) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plsdef.c
DEP_CPP_PLSDE=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plsdef.obj" : $(SOURCE) $(DEP_CPP_PLSDE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plpage.c
DEP_CPP_PLPAG=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plpage.obj" : $(SOURCE) $(DEP_CPP_PLPAG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plot3d.c
DEP_CPP_PLOT3=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plot3d.obj" : $(SOURCE) $(DEP_CPP_PLOT3) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plmeta.c
DEP_CPP_PLMET=\
	".\..\..\..\..\tmp\plDevs.h"\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\metadefs.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plmeta.obj" : $(SOURCE) $(DEP_CPP_PLMET) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plmap.c
DEP_CPP_PLMAP=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plmap.obj" : $(SOURCE) $(DEP_CPP_PLMAP) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plline.c
DEP_CPP_PLLIN=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plline.obj" : $(SOURCE) $(DEP_CPP_PLLIN) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plhist.c
DEP_CPP_PLHIS=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plhist.obj" : $(SOURCE) $(DEP_CPP_PLHIS) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plfill.c

!IF  "$(CFG)" == "plpdll - Win32 Release"

DEP_CPP_PLFIL=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plfill.obj" : $(SOURCE) $(DEP_CPP_PLFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "plpdll - Win32 Debug"

DEP_CPP_PLFIL=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	
NODEP_CPP_PLFIL=\
	"..\..\..\..\tmp\n"\
	"..\..\..\..\tmp\0"\
	"..\..\..\..\tmp\plP_fill"\
	"..\..\..\..\tmp\void"\
	"..\..\..\..\tmp\("\
	

"$(INTDIR)\plfill.obj" : $(SOURCE) $(DEP_CPP_PLFIL) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\pldtik.c
DEP_CPP_PLDTI=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\pldtik.obj" : $(SOURCE) $(DEP_CPP_PLDTI) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plcvt.c
DEP_CPP_PLCVT=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plcvt.obj" : $(SOURCE) $(DEP_CPP_PLCVT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plctrl.c
DEP_CPP_PLCTR=\
	".\..\..\..\..\tmp\plplotP.h"\
	{$(INCLUDE)}"\sys\TYPES.H"\
	{$(INCLUDE)}"\sys\STAT.H"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plctrl.obj" : $(SOURCE) $(DEP_CPP_PLCTR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plcore.c
DEP_CPP_PLCOR=\
	".\..\..\..\..\tmp\plcore.h"\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\plDevs.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plcore.obj" : $(SOURCE) $(DEP_CPP_PLCOR) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plcont.c
DEP_CPP_PLCON=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plcont.obj" : $(SOURCE) $(DEP_CPP_PLCON) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plconsole.cpp

"$(INTDIR)\plconsole.obj" : $(SOURCE) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plbuf.c
DEP_CPP_PLBUF=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\metadefs.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plbuf.obj" : $(SOURCE) $(DEP_CPP_PLBUF) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plbox.c
DEP_CPP_PLBOX=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plbox.obj" : $(SOURCE) $(DEP_CPP_PLBOX) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plargs.c
DEP_CPP_PLARG=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\plargs.obj" : $(SOURCE) $(DEP_CPP_PLARG) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\pdfutils.c
DEP_CPP_PDFUT=\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\pdfutils.obj" : $(SOURCE) $(DEP_CPP_PDFUT) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\null.c
DEP_CPP_NULL_=\
	".\..\..\..\..\tmp\plDevs.h"\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\null.obj" : $(SOURCE) $(DEP_CPP_NULL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\hpgl.c
DEP_CPP_HPGL_=\
	".\..\..\..\..\tmp\plDevs.h"\
	".\..\..\..\..\tmp\plplotP.h"\
	".\..\..\..\..\tmp\drivers.h"\
	".\..\..\..\..\tmp\plConfig.h"\
	".\..\..\..\..\tmp\plplot.h"\
	".\..\..\..\..\tmp\plstrm.h"\
	".\..\..\..\..\tmp\pldebug.h"\
	".\..\..\..\..\tmp\pdf.h"\
	

"$(INTDIR)\hpgl.obj" : $(SOURCE) $(DEP_CPP_HPGL_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plplotd.def

!IF  "$(CFG)" == "plpdll - Win32 Release"

!ELSEIF  "$(CFG)" == "plpdll - Win32 Debug"

!ENDIF

# End Source File
################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\plplot.rc
DEP_RSC_PLPLO=\
	".\..\..\..\..\tmp\plplot.ico"\
	

!IF  "$(CFG)" == "plpdll - Win32 Release"


"$(INTDIR)\plplot.res" : $(SOURCE) $(DEP_RSC_PLPLO) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/plplot.res" /i "..\..\..\..\tmp" /d "NDEBUG"\
 $(SOURCE)


!ELSEIF  "$(CFG)" == "plpdll - Win32 Debug"


"$(INTDIR)\plplot.res" : $(SOURCE) $(DEP_RSC_PLPLO) "$(INTDIR)"
   $(RSC) /l 0x409 /fo"$(INTDIR)/plplot.res" /i "..\..\..\..\tmp" /d "_DEBUG"\
 $(SOURCE)


!ENDIF

# End Source File
# End Target
# End Project
################################################################################

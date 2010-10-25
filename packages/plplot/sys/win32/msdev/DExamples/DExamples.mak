# Microsoft Developer Studio Generated NMAKE File, Format Version 4.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

!IF "$(CFG)" == ""
CFG=x01d - Win32 Release
!MESSAGE No configuration specified.  Defaulting to x19d - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "DExamples - Win32 Release" && "$(CFG)" !=\
 "DExamples - Win32 Debug" && "$(CFG)" != "x01d - Win32 Release" && "$(CFG)" !=\
 "x01d - Win32 Debug" && "$(CFG)" != "x02d - Win32 Release" && "$(CFG)" !=\
 "x02d - Win32 Debug" && "$(CFG)" != "x03d - Win32 Release" && "$(CFG)" !=\
 "x03d - Win32 Debug" && "$(CFG)" != "x04d - Win32 Release" && "$(CFG)" !=\
 "x04d - Win32 Debug" && "$(CFG)" != "x05d - Win32 Release" && "$(CFG)" !=\
 "x05d - Win32 Debug" && "$(CFG)" != "x06d - Win32 Release" && "$(CFG)" !=\
 "x06d - Win32 Debug" && "$(CFG)" != "x07d - Win32 Release" && "$(CFG)" !=\
 "x07d - Win32 Debug" && "$(CFG)" != "x08d - Win32 Release" && "$(CFG)" !=\
 "x08d - Win32 Debug" && "$(CFG)" != "x09d - Win32 Release" && "$(CFG)" !=\
 "x09d - Win32 Debug" && "$(CFG)" != "x10d - Win32 Release" && "$(CFG)" !=\
 "x10d - Win32 Debug" && "$(CFG)" != "x11d - Win32 Release" && "$(CFG)" !=\
 "x11d - Win32 Debug" && "$(CFG)" != "x12d - Win32 Release" && "$(CFG)" !=\
 "x12d - Win32 Debug" && "$(CFG)" != "x13d - Win32 Release" && "$(CFG)" !=\
 "x13d - Win32 Debug" && "$(CFG)" != "x15d - Win32 Release" && "$(CFG)" !=\
 "x15d - Win32 Debug" && "$(CFG)" != "x16d - Win32 Release" && "$(CFG)" !=\
 "x16d - Win32 Debug" && "$(CFG)" != "x18d - Win32 Release" && "$(CFG)" !=\
 "x18d - Win32 Debug" && "$(CFG)" != "x19d - Win32 Release" && "$(CFG)" !=\
 "x19d - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE on this makefile
!MESSAGE by defining the macro CFG on the command line.  For example:
!MESSAGE 
!MESSAGE NMAKE /f "DExamples.mak" CFG="x19d - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "DExamples - Win32 Release" (based on\
 "Win32 (x86) Console Application")
!MESSAGE "DExamples - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x01d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x01d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x02d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x02d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x03d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x03d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x04d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x04d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x05d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x05d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x06d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x06d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x07d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x07d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x08d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x08d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x09d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x09d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x10d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x10d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x11d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x11d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x12d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x12d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x13d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x13d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x15d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x15d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x16d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x16d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x18d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x18d - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE "x19d - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "x19d - Win32 Debug" (based on "Win32 (x86) Console Application")
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
RSC=rc.exe
CPP=cl.exe

!IF  "$(CFG)" == "DExamples - Win32 Release"

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

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/DExamples.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/DExamples.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/DExamples.pdb" /machine:I386 /out:"$(OUTDIR)/DExamples.exe" 
LINK32_OBJS=

!ELSEIF  "$(CFG)" == "DExamples - Win32 Debug"

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

ALL : 

CLEAN : 
	-@erase 

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE"\
 /Fp"$(INTDIR)/DExamples.pch" /D"__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c
CPP_OBJS=.\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/DExamples.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/DExamples.pdb" /debug /machine:I386\
 /out:"$(OUTDIR)/DExamples.exe" 
LINK32_OBJS=

!ELSEIF  "$(CFG)" == "x01d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x01d\Release"
# PROP BASE Intermediate_Dir "x01d\Release"
# PROP BASE Target_Dir "x01d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x01d\Release"
# PROP Intermediate_Dir "x01d\Release"
# PROP Target_Dir "x01d"
OUTDIR=.\x01d\Release
INTDIR=.\x01d\Release

ALL : "$(OUTDIR)\x01d.exe"

CLEAN : 
	-@erase ".\x01d\Release\x01d.exe"
	-@erase ".\x01d\Release\x01c.obj"

"$(OUTDIR)" :
    if not exist "./x01d/$(NULL)" mkdir "./x01d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "NOBRAINDEAD" /D "WIN32" /D "_CONSOLE" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "NOBRAINDEAD" /D "WIN32" /D\
 "_CONSOLE" /Fp"$(INTDIR)/x01d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x01d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x01d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x01d.pdb" /machine:I386 /out:"$(OUTDIR)/x01d.exe" 
LINK32_OBJS= \
	".\x01d\Release\x01c.obj" \
	"..\plpdll\Release\plplotd.lib"

"$(OUTDIR)\x01d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x01d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x01d\Debug"
# PROP BASE Intermediate_Dir "x01d\Debug"
# PROP BASE Target_Dir "x01d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x01d\Debug"
# PROP Intermediate_Dir "x01d\Debug"
# PROP Target_Dir "x01d"
OUTDIR=.\x01d\Debug
INTDIR=.\x01d\Debug

ALL : "$(OUTDIR)\x01d.exe"

CLEAN : 
	-@erase ".\x01d\Debug\x01d.exe"
	-@erase ".\x01d\Debug\x01c.obj"
	-@erase ".\x01d\Debug\x01d.ilk"
	-@erase ".\x01d\Debug\x01d.pdb"
	-@erase ".\x01d\Debug\vc40.pdb"
	-@erase ".\x01d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x01d/$(NULL)" mkdir "./x01d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x01d.pch"  /D "__PLDLL_H__"/YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c
 
CPP_OBJS=.\x01d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x01d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x01d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x01d.exe" 
LINK32_OBJS= \
	".\x01d\Debug\x01c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x01d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x02d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x02d\Release"
# PROP BASE Intermediate_Dir "x02d\Release"
# PROP BASE Target_Dir "x02d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x02d\Release"
# PROP Intermediate_Dir "x02d\Release"
# PROP Target_Dir "x02d"
OUTDIR=.\x02d\Release
INTDIR=.\x02d\Release

ALL : "$(OUTDIR)\x02d.exe"

CLEAN : 
	-@erase ".\x02d\Release\x02d.exe"
	-@erase ".\x02d\Release\x02c.obj"

"$(OUTDIR)" :
    if not exist "./x02d/$(NULL)" mkdir "./x02d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x02d.pch"  /D "__PLDLL_H__"/YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x02d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x02d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x02d.pdb" /machine:I386 /out:"$(OUTDIR)/x02d.exe" 
LINK32_OBJS= \
	".\x02d\Release\x02c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x02d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x02d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x02d\Debug"
# PROP BASE Intermediate_Dir "x02d\Debug"
# PROP BASE Target_Dir "x02d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x02d\Debug"
# PROP Intermediate_Dir "x02d\Debug"
# PROP Target_Dir "x02d"
OUTDIR=.\x02d\Debug
INTDIR=.\x02d\Debug

ALL : "$(OUTDIR)\x02d.exe"

CLEAN : 
	-@erase ".\x02d\Debug\x02d.exe"
	-@erase ".\x02d\Debug\x02c.obj"
	-@erase ".\x02d\Debug\x02d.ilk"
	-@erase ".\x02d\Debug\x02d.pdb"
	-@erase ".\x02d\Debug\vc40.pdb"
	-@erase ".\x02d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x02d/$(NULL)" mkdir "./x02d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x02d.pch"  /D "__PLDLL_H__"/YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x02d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x02d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x02d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x02d.exe" 
LINK32_OBJS= \
	".\x02d\Debug\x02c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x02d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x03d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x03d\Release"
# PROP BASE Intermediate_Dir "x03d\Release"
# PROP BASE Target_Dir "x03d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x03d\Release"
# PROP Intermediate_Dir "x03d\Release"
# PROP Target_Dir "x03d"
OUTDIR=.\x03d\Release
INTDIR=.\x03d\Release

ALL : "$(OUTDIR)\x03d.exe"

CLEAN : 
	-@erase ".\x03d\Release\x03d.exe"
	-@erase ".\x03d\Release\x03c.obj"

"$(OUTDIR)" :
    if not exist "./x03d/$(NULL)" mkdir "./x03d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x03d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x03d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x03d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x03d.pdb" /machine:I386 /out:"$(OUTDIR)/x03d.exe" 
LINK32_OBJS= \
	".\x03d\Release\x03c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x03d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x03d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x03d\Debug"
# PROP BASE Intermediate_Dir "x03d\Debug"
# PROP BASE Target_Dir "x03d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x03d\Debug"
# PROP Intermediate_Dir "x03d\Debug"
# PROP Target_Dir "x03d"
OUTDIR=.\x03d\Debug
INTDIR=.\x03d\Debug

ALL : "$(OUTDIR)\x03d.exe"

CLEAN : 
	-@erase ".\x03d\Debug\x03d.exe"
	-@erase ".\x03d\Debug\x03c.obj"
	-@erase ".\x03d\Debug\x03d.ilk"
	-@erase ".\x03d\Debug\x03d.pdb"
	-@erase ".\x03d\Debug\vc40.pdb"
	-@erase ".\x03d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x03d/$(NULL)" mkdir "./x03d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x03d.pch"/D "__PLDLL_H__"  /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x03d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x03d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x03d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x03d.exe" 
LINK32_OBJS= \
	".\x03d\Debug\x03c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x03d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x04d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x04d\Release"
# PROP BASE Intermediate_Dir "x04d\Release"
# PROP BASE Target_Dir "x04d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x04d\Release"
# PROP Intermediate_Dir "x04d\Release"
# PROP Target_Dir "x04d"
OUTDIR=.\x04d\Release
INTDIR=.\x04d\Release

ALL : "$(OUTDIR)\x04d.exe"

CLEAN : 
	-@erase ".\x04d\Release\x04d.exe"
	-@erase ".\x04d\Release\x04c.obj"

"$(OUTDIR)" :
    if not exist "./x04d/$(NULL)" mkdir "./x04d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x04d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x04d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x04d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x04d.pdb" /machine:I386 /out:"$(OUTDIR)/x04d.exe" 
LINK32_OBJS= \
	".\x04d\Release\x04c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x04d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x04d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x04d\Debug"
# PROP BASE Intermediate_Dir "x04d\Debug"
# PROP BASE Target_Dir "x04d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x04d\Debug"
# PROP Intermediate_Dir "x04d\Debug"
# PROP Target_Dir "x04d"
OUTDIR=.\x04d\Debug
INTDIR=.\x04d\Debug

ALL : "$(OUTDIR)\x04d.exe"

CLEAN : 
	-@erase ".\x04d\Debug\x04d.exe"
	-@erase ".\x04d\Debug\x04c.obj"
	-@erase ".\x04d\Debug\x04d.ilk"
	-@erase ".\x04d\Debug\x04d.pdb"
	-@erase ".\x04d\Debug\vc40.pdb"
	-@erase ".\x04d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x04d/$(NULL)" mkdir "./x04d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x04d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x04d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x04d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x04d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x04d.exe" 
LINK32_OBJS= \
	".\x04d\Debug\x04c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x04d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x05d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x05d\Release"
# PROP BASE Intermediate_Dir "x05d\Release"
# PROP BASE Target_Dir "x05d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x05d\Release"
# PROP Intermediate_Dir "x05d\Release"
# PROP Target_Dir "x05d"
OUTDIR=.\x05d\Release
INTDIR=.\x05d\Release

ALL : "$(OUTDIR)\x05d.exe"

CLEAN : 
	-@erase ".\x05d\Release\x05d.exe"
	-@erase ".\x05d\Release\x05c.obj"

"$(OUTDIR)" :
    if not exist "./x05d/$(NULL)" mkdir "./x05d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x05d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x05d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x05d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x05d.pdb" /machine:I386 /out:"$(OUTDIR)/x05d.exe" 
LINK32_OBJS= \
	".\x05d\Release\x05c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x05d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x05d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x05d\Debug"
# PROP BASE Intermediate_Dir "x05d\Debug"
# PROP BASE Target_Dir "x05d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x05d\Debug"
# PROP Intermediate_Dir "x05d\Debug"
# PROP Target_Dir "x05d"
OUTDIR=.\x05d\Debug
INTDIR=.\x05d\Debug

ALL : "$(OUTDIR)\x05d.exe"

CLEAN : 
	-@erase ".\x05d\Debug\x05d.exe"
	-@erase ".\x05d\Debug\x05c.obj"
	-@erase ".\x05d\Debug\x05d.ilk"
	-@erase ".\x05d\Debug\x05d.pdb"
	-@erase ".\x05d\Debug\vc40.pdb"
	-@erase ".\x05d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x05d/$(NULL)" mkdir "./x05d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x05d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x05d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x05d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x05d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x05d.exe" 
LINK32_OBJS= \
	".\x05d\Debug\x05c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x05d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x06d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x06d\Release"
# PROP BASE Intermediate_Dir "x06d\Release"
# PROP BASE Target_Dir "x06d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x06d\Release"
# PROP Intermediate_Dir "x06d\Release"
# PROP Target_Dir "x06d"
OUTDIR=.\x06d\Release
INTDIR=.\x06d\Release

ALL : "$(OUTDIR)\x06d.exe"

CLEAN : 
	-@erase ".\x06d\Release\x06d.exe"
	-@erase ".\x06d\Release\x06c.obj"

"$(OUTDIR)" :
    if not exist "./x06d/$(NULL)" mkdir "./x06d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x06d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x06d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x06d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x06d.pdb" /machine:I386 /out:"$(OUTDIR)/x06d.exe" 
LINK32_OBJS= \
	".\x06d\Release\x06c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x06d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x06d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x06d\Debug"
# PROP BASE Intermediate_Dir "x06d\Debug"
# PROP BASE Target_Dir "x06d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x06d\Debug"
# PROP Intermediate_Dir "x06d\Debug"
# PROP Target_Dir "x06d"
OUTDIR=.\x06d\Debug
INTDIR=.\x06d\Debug

ALL : "$(OUTDIR)\x06d.exe"

CLEAN : 
	-@erase ".\x06d\Debug\x06d.exe"
	-@erase ".\x06d\Debug\x06c.obj"
	-@erase ".\x06d\Debug\x06d.ilk"
	-@erase ".\x06d\Debug\x06d.pdb"
	-@erase ".\x06d\Debug\vc40.pdb"
	-@erase ".\x06d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x06d/$(NULL)" mkdir "./x06d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x06d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x06d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x06d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x06d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x06d.exe" 
LINK32_OBJS= \
	".\x06d\Debug\x06c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x06d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x07d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x07d\Release"
# PROP BASE Intermediate_Dir "x07d\Release"
# PROP BASE Target_Dir "x07d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x07d\Release"
# PROP Intermediate_Dir "x07d\Release"
# PROP Target_Dir "x07d"
OUTDIR=.\x07d\Release
INTDIR=.\x07d\Release

ALL : "$(OUTDIR)\x07d.exe"

CLEAN : 
	-@erase ".\x07d\Release\x07d.exe"
	-@erase ".\x07d\Release\x07c.obj"

"$(OUTDIR)" :
    if not exist "./x07d/$(NULL)" mkdir "./x07d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x07d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x07d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x07d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x07d.pdb" /machine:I386 /out:"$(OUTDIR)/x07d.exe" 
LINK32_OBJS= \
	".\x07d\Release\x07c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x07d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x07d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x07d\Debug"
# PROP BASE Intermediate_Dir "x07d\Debug"
# PROP BASE Target_Dir "x07d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x07d\Debug"
# PROP Intermediate_Dir "x07d\Debug"
# PROP Target_Dir "x07d"
OUTDIR=.\x07d\Debug
INTDIR=.\x07d\Debug

ALL : "$(OUTDIR)\x07d.exe"

CLEAN : 
	-@erase ".\x07d\Debug\x07d.exe"
	-@erase ".\x07d\Debug\x07c.obj"
	-@erase ".\x07d\Debug\x07d.ilk"
	-@erase ".\x07d\Debug\x07d.pdb"
	-@erase ".\x07d\Debug\vc40.pdb"
	-@erase ".\x07d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x07d/$(NULL)" mkdir "./x07d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x07d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x07d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x07d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x07d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x07d.exe" 
LINK32_OBJS= \
	".\x07d\Debug\x07c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x07d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x08d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x08d\Release"
# PROP BASE Intermediate_Dir "x08d\Release"
# PROP BASE Target_Dir "x08d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x08d\Release"
# PROP Intermediate_Dir "x08d\Release"
# PROP Target_Dir "x08d"
OUTDIR=.\x08d\Release
INTDIR=.\x08d\Release

ALL : "$(OUTDIR)\x08d.exe"

CLEAN : 
	-@erase ".\x08d\Release\x08d.exe"
	-@erase ".\x08d\Release\x08c.obj"

"$(OUTDIR)" :
    if not exist "./x08d/$(NULL)" mkdir "./x08d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x08d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x08d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x08d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x08d.pdb" /machine:I386 /out:"$(OUTDIR)/x08d.exe" 
LINK32_OBJS= \
	".\x08d\Release\x08c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x08d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x08d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x08d\Debug"
# PROP BASE Intermediate_Dir "x08d\Debug"
# PROP BASE Target_Dir "x08d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x08d\Debug"
# PROP Intermediate_Dir "x08d\Debug"
# PROP Target_Dir "x08d"
OUTDIR=.\x08d\Debug
INTDIR=.\x08d\Debug

ALL : "$(OUTDIR)\x08d.exe"

CLEAN : 
	-@erase ".\x08d\Debug\x08d.exe"
	-@erase ".\x08d\Debug\x08c.obj"
	-@erase ".\x08d\Debug\x08d.ilk"
	-@erase ".\x08d\Debug\x08d.pdb"
	-@erase ".\x08d\Debug\vc40.pdb"
	-@erase ".\x08d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x08d/$(NULL)" mkdir "./x08d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x08d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x08d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x08d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x08d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x08d.exe" 
LINK32_OBJS= \
	".\x08d\Debug\x08c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x08d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x09d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x09d\Release"
# PROP BASE Intermediate_Dir "x09d\Release"
# PROP BASE Target_Dir "x09d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x09d\Release"
# PROP Intermediate_Dir "x09d\Release"
# PROP Target_Dir "x09d"
OUTDIR=.\x09d\Release
INTDIR=.\x09d\Release

ALL : "$(OUTDIR)\x09d.exe"

CLEAN : 
	-@erase ".\x09d\Release\x09d.exe"
	-@erase ".\x09d\Release\x09c.obj"

"$(OUTDIR)" :
    if not exist "./x09d/$(NULL)" mkdir "./x09d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x09d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x09d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x09d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x09d.pdb" /machine:I386 /out:"$(OUTDIR)/x09d.exe" 
LINK32_OBJS= \
	".\x09d\Release\x09c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x09d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x09d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x09d\Debug"
# PROP BASE Intermediate_Dir "x09d\Debug"
# PROP BASE Target_Dir "x09d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x09d\Debug"
# PROP Intermediate_Dir "x09d\Debug"
# PROP Target_Dir "x09d"
OUTDIR=.\x09d\Debug
INTDIR=.\x09d\Debug

ALL : "$(OUTDIR)\x09d.exe"

CLEAN : 
	-@erase ".\x09d\Debug\x09d.exe"
	-@erase ".\x09d\Debug\x09c.obj"
	-@erase ".\x09d\Debug\x09d.ilk"
	-@erase ".\x09d\Debug\x09d.pdb"
	-@erase ".\x09d\Debug\vc40.pdb"
	-@erase ".\x09d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x09d/$(NULL)" mkdir "./x09d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x09d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x09d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x09d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x09d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x09d.exe" 
LINK32_OBJS= \
	".\x09d\Debug\x09c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x09d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x10d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x10d\Release"
# PROP BASE Intermediate_Dir "x10d\Release"
# PROP BASE Target_Dir "x10d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x10d\Release"
# PROP Intermediate_Dir "x10d\Release"
# PROP Target_Dir "x10d"
OUTDIR=.\x10d\Release
INTDIR=.\x10d\Release

ALL : "$(OUTDIR)\x10d.exe"

CLEAN : 
	-@erase ".\x10d\Release\x10d.exe"
	-@erase ".\x10d\Release\x10c.obj"

"$(OUTDIR)" :
    if not exist "./x10d/$(NULL)" mkdir "./x10d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x10d.pch" /YX /Fo"$(INTDIR)/" /c 
CPP_OBJS=.\x10d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x10d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x10d.pdb" /machine:I386 /out:"$(OUTDIR)/x10d.exe" 
LINK32_OBJS= \
	".\x10d\Release\x10c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x10d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x10d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x10d\Debug"
# PROP BASE Intermediate_Dir "x10d\Debug"
# PROP BASE Target_Dir "x10d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x10d\Debug"
# PROP Intermediate_Dir "x10d\Debug"
# PROP Target_Dir "x10d"
OUTDIR=.\x10d\Debug
INTDIR=.\x10d\Debug

ALL : "$(OUTDIR)\x10d.exe"

CLEAN : 
	-@erase ".\x10d\Debug\x10d.exe"
	-@erase ".\x10d\Debug\x10c.obj"
	-@erase ".\x10d\Debug\x10d.ilk"
	-@erase ".\x10d\Debug\x10d.pdb"
	-@erase ".\x10d\Debug\vc40.pdb"
	-@erase ".\x10d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x10d/$(NULL)" mkdir "./x10d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x10d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x10d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x10d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x10d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x10d.exe" 
LINK32_OBJS= \
	".\x10d\Debug\x10c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x10d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x11d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x11d\Release"
# PROP BASE Intermediate_Dir "x11d\Release"
# PROP BASE Target_Dir "x11d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x11d\Release"
# PROP Intermediate_Dir "x11d\Release"
# PROP Target_Dir "x11d"
OUTDIR=.\x11d\Release
INTDIR=.\x11d\Release

ALL : "$(OUTDIR)\x11d.exe"

CLEAN : 
	-@erase ".\x11d\Release\x11d.exe"
	-@erase ".\x11d\Release\x11c.obj"

"$(OUTDIR)" :
    if not exist "./x11d/$(NULL)" mkdir "./x11d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x11d.pch"/D "__PLDLL_H__"  /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x11d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x11d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x11d.pdb" /machine:I386 /out:"$(OUTDIR)/x11d.exe" 
LINK32_OBJS= \
	".\x11d\Release\x11c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x11d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x11d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x11d\Debug"
# PROP BASE Intermediate_Dir "x11d\Debug"
# PROP BASE Target_Dir "x11d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x11d\Debug"
# PROP Intermediate_Dir "x11d\Debug"
# PROP Target_Dir "x11d"
OUTDIR=.\x11d\Debug
INTDIR=.\x11d\Debug

ALL : "$(OUTDIR)\x11d.exe"

CLEAN : 
	-@erase ".\x11d\Debug\x11d.exe"
	-@erase ".\x11d\Debug\x11c.obj"
	-@erase ".\x11d\Debug\x11d.ilk"
	-@erase ".\x11d\Debug\x11d.pdb"
	-@erase ".\x11d\Debug\vc40.pdb"
	-@erase ".\x11d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x11d/$(NULL)" mkdir "./x11d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x11d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x11d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x11d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x11d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x11d.exe" 
LINK32_OBJS= \
	".\x11d\Debug\x11c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x11d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x12d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x12d\Release"
# PROP BASE Intermediate_Dir "x12d\Release"
# PROP BASE Target_Dir "x12d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x12d\Release"
# PROP Intermediate_Dir "x12d\Release"
# PROP Target_Dir "x12d"
OUTDIR=.\x12d\Release
INTDIR=.\x12d\Release

ALL : "$(OUTDIR)\x12d.exe"

CLEAN : 
	-@erase ".\x12d\Release\x12d.exe"
	-@erase ".\x12d\Release\x12c.obj"

"$(OUTDIR)" :
    if not exist "./x12d/$(NULL)" mkdir "./x12d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x12d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x12d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x12d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x12d.pdb" /machine:I386 /out:"$(OUTDIR)/x12d.exe" 
LINK32_OBJS= \
	".\x12d\Release\x12c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x12d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x12d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x12d\Debug"
# PROP BASE Intermediate_Dir "x12d\Debug"
# PROP BASE Target_Dir "x12d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x12d\Debug"
# PROP Intermediate_Dir "x12d\Debug"
# PROP Target_Dir "x12d"
OUTDIR=.\x12d\Debug
INTDIR=.\x12d\Debug

ALL : "$(OUTDIR)\x12d.exe"

CLEAN : 
	-@erase ".\x12d\Debug\x12d.exe"
	-@erase ".\x12d\Debug\x12c.obj"
	-@erase ".\x12d\Debug\x12d.ilk"
	-@erase ".\x12d\Debug\x12d.pdb"
	-@erase ".\x12d\Debug\vc40.pdb"
	-@erase ".\x12d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x12d/$(NULL)" mkdir "./x12d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x12d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x12d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x12d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x12d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x12d.exe" 
LINK32_OBJS= \
	".\x12d\Debug\x12c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x12d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x13d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x13d\Release"
# PROP BASE Intermediate_Dir "x13d\Release"
# PROP BASE Target_Dir "x13d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x13d\Release"
# PROP Intermediate_Dir "x13d\Release"
# PROP Target_Dir "x13d"
OUTDIR=.\x13d\Release
INTDIR=.\x13d\Release

ALL : "$(OUTDIR)\x13d.exe"

CLEAN : 
	-@erase ".\x13d\Release\x13d.exe"
	-@erase ".\x13d\Release\x13c.obj"

"$(OUTDIR)" :
    if not exist "./x13d/$(NULL)" mkdir "./x13d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x13d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x13d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x13d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x13d.pdb" /machine:I386 /out:"$(OUTDIR)/x13d.exe" 
LINK32_OBJS= \
	".\x13d\Release\x13c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x13d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x13d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x13d\Debug"
# PROP BASE Intermediate_Dir "x13d\Debug"
# PROP BASE Target_Dir "x13d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x13d\Debug"
# PROP Intermediate_Dir "x13d\Debug"
# PROP Target_Dir "x13d"
OUTDIR=.\x13d\Debug
INTDIR=.\x13d\Debug

ALL : "$(OUTDIR)\x13d.exe"

CLEAN : 
	-@erase ".\x13d\Debug\x13d.exe"
	-@erase ".\x13d\Debug\x13c.obj"
	-@erase ".\x13d\Debug\x13d.ilk"
	-@erase ".\x13d\Debug\x13d.pdb"
	-@erase ".\x13d\Debug\vc40.pdb"
	-@erase ".\x13d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x13d/$(NULL)" mkdir "./x13d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x13d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x13d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x13d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x13d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x13d.exe" 
LINK32_OBJS= \
	".\x13d\Debug\x13c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x13d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x15d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x15d\Release"
# PROP BASE Intermediate_Dir "x15d\Release"
# PROP BASE Target_Dir "x15d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x15d\Release"
# PROP Intermediate_Dir "x15d\Release"
# PROP Target_Dir "x15d"
OUTDIR=.\x15d\Release
INTDIR=.\x15d\Release

ALL : "$(OUTDIR)\x15d.exe"

CLEAN : 
	-@erase ".\x15d\Release\x15d.exe"
	-@erase ".\x15d\Release\x15c.obj"

"$(OUTDIR)" :
    if not exist "./x15d/$(NULL)" mkdir "./x15d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x15d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x15d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x15d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x15d.pdb" /machine:I386 /out:"$(OUTDIR)/x15d.exe" 
LINK32_OBJS= \
	".\x15d\Release\x15c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x15d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x15d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x15d\Debug"
# PROP BASE Intermediate_Dir "x15d\Debug"
# PROP BASE Target_Dir "x15d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x15d\Debug"
# PROP Intermediate_Dir "x15d\Debug"
# PROP Target_Dir "x15d"
OUTDIR=.\x15d\Debug
INTDIR=.\x15d\Debug

ALL : "$(OUTDIR)\x15d.exe"

CLEAN : 
	-@erase ".\x15d\Debug\x15d.exe"
	-@erase ".\x15d\Debug\x15c.obj"
	-@erase ".\x15d\Debug\x15d.ilk"
	-@erase ".\x15d\Debug\x15d.pdb"
	-@erase ".\x15d\Debug\vc40.pdb"
	-@erase ".\x15d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x15d/$(NULL)" mkdir "./x15d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x15d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x15d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x15d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x15d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x15d.exe" 
LINK32_OBJS= \
	".\x15d\Debug\x15c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x15d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x16d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x16d\Release"
# PROP BASE Intermediate_Dir "x16d\Release"
# PROP BASE Target_Dir "x16d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x16d\Release"
# PROP Intermediate_Dir "x16d\Release"
# PROP Target_Dir "x16d"
OUTDIR=.\x16d\Release
INTDIR=.\x16d\Release

ALL : "$(OUTDIR)\x16d.exe"

CLEAN : 
	-@erase ".\x16d\Release\x16d.exe"
	-@erase ".\x16d\Release\x16c.obj"

"$(OUTDIR)" :
    if not exist "./x16d/$(NULL)" mkdir "./x16d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x16d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x16d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x16d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x16d.pdb" /machine:I386 /out:"$(OUTDIR)/x16d.exe" 
LINK32_OBJS= \
	".\x16d\Release\x16c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x16d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x16d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x16d\Debug"
# PROP BASE Intermediate_Dir "x16d\Debug"
# PROP BASE Target_Dir "x16d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x16d\Debug"
# PROP Intermediate_Dir "x16d\Debug"
# PROP Target_Dir "x16d"
OUTDIR=.\x16d\Debug
INTDIR=.\x16d\Debug

ALL : "$(OUTDIR)\x16d.exe"

CLEAN : 
	-@erase ".\x16d\Debug\x16d.exe"
	-@erase ".\x16d\Debug\x16c.obj"
	-@erase ".\x16d\Debug\x16d.ilk"
	-@erase ".\x16d\Debug\x16d.pdb"
	-@erase ".\x16d\Debug\vc40.pdb"
	-@erase ".\x16d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x16d/$(NULL)" mkdir "./x16d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x16d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x16d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x16d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x16d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x16d.exe" 
LINK32_OBJS= \
	".\x16d\Debug\x16c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x16d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x18d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x18d\Release"
# PROP BASE Intermediate_Dir "x18d\Release"
# PROP BASE Target_Dir "x18d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x18d\Release"
# PROP Intermediate_Dir "x18d\Release"
# PROP Target_Dir "x18d"
OUTDIR=.\x18d\Release
INTDIR=.\x18d\Release

ALL : "$(OUTDIR)\x18d.exe"

CLEAN : 
	-@erase ".\x18d\Release\x18d.exe"
	-@erase ".\x18d\Release\x18c.obj"

"$(OUTDIR)" :
    if not exist "./x18d/$(NULL)" mkdir "./x18d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /Fp"$(INTDIR)/x18d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x18d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x18d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x18d.pdb" /machine:I386 /out:"$(OUTDIR)/x18d.exe" 
LINK32_OBJS= \
	".\x18d\Release\x18c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x18d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x18d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x18d\Debug"
# PROP BASE Intermediate_Dir "x18d\Debug"
# PROP BASE Target_Dir "x18d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x18d\Debug"
# PROP Intermediate_Dir "x18d\Debug"
# PROP Target_Dir "x18d"
OUTDIR=.\x18d\Debug
INTDIR=.\x18d\Debug

ALL : "$(OUTDIR)\x18d.exe"

CLEAN : 
	-@erase ".\x18d\Debug\x18d.exe"
	-@erase ".\x18d\Debug\x18c.obj"
	-@erase ".\x18d\Debug\x18d.ilk"
	-@erase ".\x18d\Debug\x18d.pdb"
	-@erase ".\x18d\Debug\vc40.pdb"
	-@erase ".\x18d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x18d/$(NULL)" mkdir "./x18d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x18d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c\
 
CPP_OBJS=.\x18d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x18d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x18d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x18d.exe" 
LINK32_OBJS= \
	".\x18d\Debug\x18c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x18d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x19d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "x19d\Release"
# PROP BASE Intermediate_Dir "x19d\Release"
# PROP BASE Target_Dir "x19d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "x19d\Release"
# PROP Intermediate_Dir "x19d\Release"
# PROP Target_Dir "x19d"
OUTDIR=.\x19d\Release
INTDIR=.\x19d\Release

ALL : "$(OUTDIR)\x19d.exe"

CLEAN : 
	-@erase ".\x19d\Release\x19d.exe"
	-@erase ".\x19d\Release\x19c.obj"

"$(OUTDIR)" :
    if not exist "./x19d/$(NULL)" mkdir "./x19d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "NDEBUG" /D "WIN32" /D "_CONSOLE" /D\
 "NOBRAINDEAD" /D "__PLDLL_H__" /Fp"$(INTDIR)/x19d.pch" /YX /Fo"$(INTDIR)/" /c
CPP_OBJS=.\x19d\Release/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x19d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:no\
 /pdb:"$(OUTDIR)/x19d.pdb" /machine:I386 /out:"$(OUTDIR)/x19d.exe" 
LINK32_OBJS= \
	".\x19d\Release\x19c.obj" \
	"..\plpdll\Release\plplotd.lib"

"$(OUTDIR)\x19d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "x19d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "x19d\Debug"
# PROP BASE Intermediate_Dir "x19d\Debug"
# PROP BASE Target_Dir "x19d"
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "x19d\Debug"
# PROP Intermediate_Dir "x19d\Debug"
# PROP Target_Dir "x19d"
OUTDIR=.\x19d\Debug
INTDIR=.\x19d\Debug

ALL : "$(OUTDIR)\x19d.exe"

CLEAN : 
	-@erase ".\x19d\Debug\x19d.exe"
	-@erase ".\x19d\Debug\x19c.obj"
	-@erase ".\x19d\Debug\x19d.ilk"
	-@erase ".\x19d\Debug\x19d.pdb"
	-@erase ".\x19d\Debug\vc40.pdb"
	-@erase ".\x19d\Debug\vc40.idb"

"$(OUTDIR)" :
    if not exist "./x19d/$(NULL)" mkdir "./x19d"
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /YX /c
# ADD CPP /nologo /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE" /D "NOBRAINDEAD" /YX /c
CPP_PROJ=/nologo /MLd /W3 /Gm /GX /Zi /Od /D "_DEBUG" /D "WIN32" /D "_CONSOLE"\
 /D "NOBRAINDEAD" /Fp"$(INTDIR)/x19d.pch" /D "__PLDLL_H__" /YX /Fo"$(INTDIR)/" /Fd"$(INTDIR)/" /c
CPP_OBJS=.\x19d\Debug/
CPP_SBRS=
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
BSC32_FLAGS=/nologo /o"$(OUTDIR)/x19d.bsc" 
BSC32_SBRS=
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib\
 advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib\
 odbccp32.lib /nologo /subsystem:console /incremental:yes\
 /pdb:"$(OUTDIR)/x19d.pdb" /debug /machine:I386 /out:"$(OUTDIR)/x19d.exe" 
LINK32_OBJS= \
	".\x19d\Debug\x19c.obj" \
	"..\..\..\..\lib\plplotd.lib"

"$(OUTDIR)\x19d.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
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

# Name "DExamples - Win32 Release"
# Name "DExamples - Win32 Debug"

!IF  "$(CFG)" == "DExamples - Win32 Release"

!ELSEIF  "$(CFG)" == "DExamples - Win32 Debug"

!ENDIF 

# End Target
################################################################################
# Begin Target

# Name "x01d - Win32 Release"
# Name "x01d - Win32 Debug"

!IF  "$(CFG)" == "x01d - Win32 Release"

!ELSEIF  "$(CFG)" == "x01d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\x01c.c
DEP_CPP_X01C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plevent.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x01c.obj" : $(SOURCE) $(DEP_CPP_X01C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x01d - Win32 Release"

!ELSEIF  "$(CFG)" == "x01d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x02d - Win32 Release"
# Name "x02d - Win32 Debug"

!IF  "$(CFG)" == "x02d - Win32 Release"

!ELSEIF  "$(CFG)" == "x02d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x02c.cpp

!IF  "$(CFG)" == "x02d - Win32 Release"

DEP_CPP_X02C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x02c.obj" : $(SOURCE) $(DEP_CPP_X02C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "x02d - Win32 Debug"

DEP_CPP_X02C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x02c.obj" : $(SOURCE) $(DEP_CPP_X02C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x02d - Win32 Release"

!ELSEIF  "$(CFG)" == "x02d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x03d - Win32 Release"
# Name "x03d - Win32 Debug"

!IF  "$(CFG)" == "x03d - Win32 Release"

!ELSEIF  "$(CFG)" == "x03d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x03c.cpp
DEP_CPP_X03C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x03c.obj" : $(SOURCE) $(DEP_CPP_X03C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x03d - Win32 Release"

!ELSEIF  "$(CFG)" == "x03d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x04d - Win32 Release"
# Name "x04d - Win32 Debug"

!IF  "$(CFG)" == "x04d - Win32 Release"

!ELSEIF  "$(CFG)" == "x04d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x04c.cpp
DEP_CPP_X04C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x04c.obj" : $(SOURCE) $(DEP_CPP_X04C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x04d - Win32 Release"

!ELSEIF  "$(CFG)" == "x04d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x05d - Win32 Release"
# Name "x05d - Win32 Debug"

!IF  "$(CFG)" == "x05d - Win32 Release"

!ELSEIF  "$(CFG)" == "x05d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x05c.cpp
DEP_CPP_X05C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x05c.obj" : $(SOURCE) $(DEP_CPP_X05C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x05d - Win32 Release"

!ELSEIF  "$(CFG)" == "x05d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x06d - Win32 Release"
# Name "x06d - Win32 Debug"

!IF  "$(CFG)" == "x06d - Win32 Release"

!ELSEIF  "$(CFG)" == "x06d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x06c.cpp
DEP_CPP_X06C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x06c.obj" : $(SOURCE) $(DEP_CPP_X06C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x06d - Win32 Release"

!ELSEIF  "$(CFG)" == "x06d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x07d - Win32 Release"
# Name "x07d - Win32 Debug"

!IF  "$(CFG)" == "x07d - Win32 Release"

!ELSEIF  "$(CFG)" == "x07d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x07c.cpp

!IF  "$(CFG)" == "x07d - Win32 Release"

DEP_CPP_X07C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x07c.obj" : $(SOURCE) $(DEP_CPP_X07C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "x07d - Win32 Debug"

DEP_CPP_X07C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x07c.obj" : $(SOURCE) $(DEP_CPP_X07C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x07d - Win32 Release"

!ELSEIF  "$(CFG)" == "x07d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x08d - Win32 Release"
# Name "x08d - Win32 Debug"

!IF  "$(CFG)" == "x08d - Win32 Release"

!ELSEIF  "$(CFG)" == "x08d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x08c.cpp
DEP_CPP_X08C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x08c.obj" : $(SOURCE) $(DEP_CPP_X08C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x08d - Win32 Release"

!ELSEIF  "$(CFG)" == "x08d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x09d - Win32 Release"
# Name "x09d - Win32 Debug"

!IF  "$(CFG)" == "x09d - Win32 Release"

!ELSEIF  "$(CFG)" == "x09d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x09c.cpp
DEP_CPP_X09C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x09c.obj" : $(SOURCE) $(DEP_CPP_X09C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x09d - Win32 Release"

!ELSEIF  "$(CFG)" == "x09d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x10d - Win32 Release"
# Name "x10d - Win32 Debug"

!IF  "$(CFG)" == "x10d - Win32 Release"

!ELSEIF  "$(CFG)" == "x10d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x10c.cpp
DEP_CPP_X10C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x10c.obj" : $(SOURCE) $(DEP_CPP_X10C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x10d - Win32 Release"

!ELSEIF  "$(CFG)" == "x10d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x11d - Win32 Release"
# Name "x11d - Win32 Debug"

!IF  "$(CFG)" == "x11d - Win32 Release"

!ELSEIF  "$(CFG)" == "x11d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x11c.cpp
DEP_CPP_X11C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x11c.obj" : $(SOURCE) $(DEP_CPP_X11C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x11d - Win32 Release"

!ELSEIF  "$(CFG)" == "x11d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x12d - Win32 Release"
# Name "x12d - Win32 Debug"

!IF  "$(CFG)" == "x12d - Win32 Release"

!ELSEIF  "$(CFG)" == "x12d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x12c.cpp
DEP_CPP_X12C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x12c.obj" : $(SOURCE) $(DEP_CPP_X12C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x12d - Win32 Release"

!ELSEIF  "$(CFG)" == "x12d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x13d - Win32 Release"
# Name "x13d - Win32 Debug"

!IF  "$(CFG)" == "x13d - Win32 Release"

!ELSEIF  "$(CFG)" == "x13d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x13c.cpp
DEP_CPP_X13C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x13c.obj" : $(SOURCE) $(DEP_CPP_X13C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x13d - Win32 Release"

!ELSEIF  "$(CFG)" == "x13d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x15d - Win32 Release"
# Name "x15d - Win32 Debug"

!IF  "$(CFG)" == "x15d - Win32 Release"

!ELSEIF  "$(CFG)" == "x15d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x15c.cpp
DEP_CPP_X15C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x15c.obj" : $(SOURCE) $(DEP_CPP_X15C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x15d - Win32 Release"

!ELSEIF  "$(CFG)" == "x15d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x16d - Win32 Release"
# Name "x16d - Win32 Debug"

!IF  "$(CFG)" == "x16d - Win32 Release"

!ELSEIF  "$(CFG)" == "x16d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x16c.cpp
DEP_CPP_X16C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x16c.obj" : $(SOURCE) $(DEP_CPP_X16C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x16d - Win32 Release"

!ELSEIF  "$(CFG)" == "x16d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x18d - Win32 Release"
# Name "x18d - Win32 Debug"

!IF  "$(CFG)" == "x18d - Win32 Release"

!ELSEIF  "$(CFG)" == "x18d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=\plplot\tmp\x18c.cpp

!IF  "$(CFG)" == "x18d - Win32 Release"

DEP_CPP_X18C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x18c.obj" : $(SOURCE) $(DEP_CPP_X18C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ELSEIF  "$(CFG)" == "x18d - Win32 Debug"

DEP_CPP_X18C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x18c.obj" : $(SOURCE) $(DEP_CPP_X18C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


!ENDIF 

# End Source File
################################################################################
# Begin Source File

SOURCE=\plplot\lib\plplotd.lib

!IF  "$(CFG)" == "x18d - Win32 Release"

!ELSEIF  "$(CFG)" == "x18d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
################################################################################
# Begin Target

# Name "x19d - Win32 Release"
# Name "x19d - Win32 Debug"

!IF  "$(CFG)" == "x19d - Win32 Release"

!ELSEIF  "$(CFG)" == "x19d - Win32 Debug"

!ENDIF 

################################################################################
# Begin Source File

SOURCE=..\..\..\..\tmp\x19c.c
DEP_CPP_X19C_=\
	"..\..\..\..\tmp\plcdemos.h"\
	"..\..\..\..\tmp\plplot.h"\
	

"$(INTDIR)\x19c.obj" : $(SOURCE) $(DEP_CPP_X19C_) "$(INTDIR)"
   $(CPP) $(CPP_PROJ) $(SOURCE)


# End Source File
################################################################################
# Begin Source File

SOURCE=..\plpdll\Release\plplotd.lib

!IF  "$(CFG)" == "x19d - Win32 Release"

!ELSEIF  "$(CFG)" == "x19d - Win32 Debug"

!ENDIF 

# End Source File
# End Target
# End Project
################################################################################

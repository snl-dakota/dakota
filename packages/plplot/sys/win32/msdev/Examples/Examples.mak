!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE
NULL=nul
!ENDIF

SOURCEDIR=..\..\..\..\tmp

OUTDIR=.\Release
INTDIR=.\Release

PLPLIB=..\plplib\plplot.lib

ALL :   $(SOURCEDIR)\x01c.exe \
	$(SOURCEDIR)\x02c.exe \
	$(SOURCEDIR)\x03c.exe \
	$(SOURCEDIR)\x04c.exe \
	$(SOURCEDIR)\x05c.exe \
	$(SOURCEDIR)\x06c.exe \
	$(SOURCEDIR)\x07c.exe \
	$(SOURCEDIR)\x08c.exe \
	$(SOURCEDIR)\x09c.exe \
	$(SOURCEDIR)\x10c.exe \
	$(SOURCEDIR)\x11c.exe \
	$(SOURCEDIR)\x12c.exe \
	$(SOURCEDIR)\x13c.exe \
	$(SOURCEDIR)\x15c.exe \
	$(SOURCEDIR)\x16c.exe \
#	$(SOURCEDIR)\x17c.exe \
	$(SOURCEDIR)\x18c.exe \
	$(SOURCEDIR)\x19c.exe \
	$(SOURCEDIR)\x20c.exe


CLEAN :
	-@erase $(INTDIR)\vc60.idb
	-@erase $(INTDIR)\x01c.obj
	-@erase $(SOURCEDIR)\x01c.exe"
	-@erase $(INTDIR)\x02c.obj
	-@erase $(SOURCEDIR)\x02c.exe"
	-@erase $(INTDIR)\x03c.obj
	-@erase $(SOURCEDIR)\x03c.exe"
	-@erase $(INTDIR)\x04c.obj
	-@erase $(SOURCEDIR)\x04c.exe"
	-@erase $(INTDIR)\x05c.obj
	-@erase $(SOURCEDIR)\x05c.exe"
	-@erase $(INTDIR)\x06c.obj
	-@erase $(SOURCEDIR)\x06c.exe"
	-@erase $(INTDIR)\x07c.obj
	-@erase $(SOURCEDIR)\x07c.exe"
	-@erase $(INTDIR)\x08c.obj
	-@erase $(SOURCEDIR)\x08c.exe"
	-@erase $(INTDIR)\x09c.obj
	-@erase $(SOURCEDIR)\x09c.exe"
	-@erase $(INTDIR)\x10c.obj
	-@erase $(SOURCEDIR)\x10c.exe"
	-@erase $(INTDIR)\x11c.obj
	-@erase $(SOURCEDIR)\x11c.exe"
	-@erase $(INTDIR)\x12c.obj
	-@erase $(SOURCEDIR)\x12c.exe"
	-@erase $(INTDIR)\x13c.obj
	-@erase $(SOURCEDIR)\x13c.exe"
	-@erase $(INTDIR)\x15c.obj
	-@erase $(SOURCEDIR)\x15c.exe"
	-@erase $(INTDIR)\x16c.obj
	-@erase $(SOURCEDIR)\x16c.exe"
	-@erase $(INTDIR)\x17c.obj
	-@erase $(SOURCEDIR)\x17c.exe"
	-@erase $(INTDIR)\x18c.obj
	-@erase $(SOURCEDIR)\x18c.exe"
	-@erase $(INTDIR)\x19c.obj
	-@erase $(SOURCEDIR)\x19c.exe"
	-@erase $(INTDIR)\x20c.obj
	-@erase $(SOURCEDIR)\x20c.exe"


"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP=cl.exe
CPP_PROJ=/nologo /Ox /MD /W3 /GX /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c

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

RSC=rc.exe
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\x01c.bsc"
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /incremental:no /machine:I386
#LINK32_OBJS=$(PLPLIB) ..\..\..\..\gd\lib\bgd.lib
LINK32_OBJS=$(PLPLIB)

$(SOURCEDIR)\x01c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x01c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x01c.pdb /out:$(SOURCEDIR)\x01c.exe $(INTDIR)\x01c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x02c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x02c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x02c.pdb /out:$(SOURCEDIR)\x02c.exe $(INTDIR)\x02c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x03c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x03c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x03c.pdb /out:$(SOURCEDIR)\x03c.exe $(INTDIR)\x03c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x04c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x04c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x04c.pdb /out:$(SOURCEDIR)\x04c.exe $(INTDIR)\x04c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x05c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x05c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x05c.pdb /out:$(SOURCEDIR)\x05c.exe $(INTDIR)\x05c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x06c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x06c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x06c.pdb /out:$(SOURCEDIR)\x06c.exe $(INTDIR)\x06c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x07c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x07c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x07c.pdb /out:$(SOURCEDIR)\x07c.exe $(INTDIR)\x07c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x08c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x08c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x08c.pdb /out:$(SOURCEDIR)\x08c.exe $(INTDIR)\x08c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x09c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x09c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x09c.pdb /out:$(SOURCEDIR)\x09c.exe $(INTDIR)\x09c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x10c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x10c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x10c.pdb /out:$(SOURCEDIR)\x10c.exe $(INTDIR)\x10c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x11c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x11c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x11c.pdb /out:$(SOURCEDIR)\x11c.exe $(INTDIR)\x11c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x12c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x12c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x12c.pdb /out:$(SOURCEDIR)\x12c.exe $(INTDIR)\x12c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x13c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x13c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x13c.pdb /out:$(SOURCEDIR)\x13c.exe $(INTDIR)\x13c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x15c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x15c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x15c.pdb /out:$(SOURCEDIR)\x15c.exe $(INTDIR)\x15c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x16c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x16c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x16c.pdb /out:$(SOURCEDIR)\x16c.exe $(INTDIR)\x16c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x17c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x17c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x17c.pdb /out:$(SOURCEDIR)\x17c.exe $(INTDIR)\x17c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x18c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x18c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x18c.pdb /out:$(SOURCEDIR)\x18c.exe $(INTDIR)\x18c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x19c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x19c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x19c.pdb /out:$(SOURCEDIR)\x19c.exe $(INTDIR)\x19c.obj $(LINK32_OBJS)
<<

$(SOURCEDIR)\x20c.exe : $(OUTDIR) $(DEF_FILE) $(INTDIR)\x20c.obj $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) /pdb:$(OUTDIR)\x20c.pdb /out:$(SOURCEDIR)\x20c.exe $(INTDIR)\x20c.obj $(LINK32_OBJS)
<<



$(INTDIR)\x01c.obj : $(SOURCEDIR)\x01c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x01c.pch $(SOURCEDIR)\x01c.c

$(INTDIR)\x02c.obj : $(SOURCEDIR)\x02c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x02c.pch $(SOURCEDIR)\x02c.c

$(INTDIR)\x03c.obj : $(SOURCEDIR)\x03c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x03c.pch $(SOURCEDIR)\x03c.c

$(INTDIR)\x04c.obj : $(SOURCEDIR)\x04c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x04c.pch $(SOURCEDIR)\x04c.c

$(INTDIR)\x05c.obj : $(SOURCEDIR)\x05c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x05c.pch $(SOURCEDIR)\x05c.c

$(INTDIR)\x06c.obj : $(SOURCEDIR)\x06c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x06c.pch $(SOURCEDIR)\x06c.c

$(INTDIR)\x07c.obj : $(SOURCEDIR)\x07c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x07c.pch $(SOURCEDIR)\x07c.c

$(INTDIR)\x08c.obj : $(SOURCEDIR)\x08c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x08c.pch $(SOURCEDIR)\x08c.c

$(INTDIR)\x09c.obj : $(SOURCEDIR)\x09c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x09c.pch $(SOURCEDIR)\x09c.c

$(INTDIR)\x10c.obj : $(SOURCEDIR)\x10c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x10c.pch $(SOURCEDIR)\x10c.c

$(INTDIR)\x11c.obj : $(SOURCEDIR)\x11c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x11c.pch $(SOURCEDIR)\x11c.c

$(INTDIR)\x12c.obj : $(SOURCEDIR)\x12c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x12c.pch $(SOURCEDIR)\x12c.c

$(INTDIR)\x13c.obj : $(SOURCEDIR)\x13c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x13c.pch $(SOURCEDIR)\x13c.c

$(INTDIR)\x15c.obj : $(SOURCEDIR)\x15c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x15c.pch $(SOURCEDIR)\x15c.c

$(INTDIR)\x16c.obj : $(SOURCEDIR)\x16c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x16c.pch $(SOURCEDIR)\x16c.c

$(INTDIR)\x17c.obj : $(SOURCEDIR)\x17c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x17c.pch $(SOURCEDIR)\x17c.c

$(INTDIR)\x18c.obj : $(SOURCEDIR)\x18c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x18c.pch $(SOURCEDIR)\x18c.c

$(INTDIR)\x19c.obj : $(SOURCEDIR)\x19c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x19c.pch $(SOURCEDIR)\x19c.c

$(INTDIR)\x20c.obj : $(SOURCEDIR)\x20c.c $(INTDIR)
	$(CPP) $(CPP_PROJ) /Fp$(INTDIR)\x20c.pch $(SOURCEDIR)\x20c.c




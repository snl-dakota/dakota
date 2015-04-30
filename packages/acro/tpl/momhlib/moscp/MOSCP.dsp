# Microsoft Developer Studio Project File - Name="MOSCP" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=MOSCP - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "MOSCP.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "MOSCP.mak" CFG="MOSCP - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "MOSCP - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "MOSCP - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MOSCP - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /I "../MOMHLib++/ParetoGA" /I "../MOMHLib++/MO" /I "../MOMHLib++/HGA" /I "../MOMHLib++/SA" /I "../MOMHLib++/MOMSLS" /I "../MOMHLib++/common" /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /profile /debug /machine:I386

!ELSEIF  "$(CFG)" == "MOSCP - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../MOMHLib++/MO" /I "../MOMHLib++/HGA" /I "../MOMHLib++/SA" /I "../MOMHLib++/MOMSLS" /I "../MOMHLib++/common" /I "../MOMHLib++/ParetoGA" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "MOSCP - Win32 Release"
# Name "MOSCP - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE="..\MOMHLib++\MO\global.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\HGA\hgabase.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\HGA\immogls.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\HGA\immogls.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\HGA\mogls.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\HGA\mogls.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momethod.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momhconstrainedsolution.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momhconstrainedsolution.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MOMSLS\momsls.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MOMSLS\momsls.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\mosa.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\mosa.h"
# End Source File
# Begin Source File

SOURCE=.\MOSCP.cpp
# End Source File
# Begin Source File

SOURCE=.\moscpproblem.cpp
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\nondominatedset.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\ParetoGA\paretobase.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\ParetoGA\paretobase.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\HGA\pma.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\psa.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\psa.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\sabase.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\smosa.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\smosa.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\solution.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\solutionsset.cpp"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tbnode.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tbtree.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\ParetoGA\tindexedpoint.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tlistset.cpp"
# End Source File
# Begin Source File

SOURCE=.\tmoscpsolution.cpp
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tmotools.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tquadnode.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tquadtree.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\Common\trandomgenerator.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\weights.cpp"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE="..\MOMHLib++\MO\global.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\HGA\hgabase.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momethod.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momhsolution.h"
# End Source File
# Begin Source File

SOURCE=.\moscpproblem.h
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\nondominatedset.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\HGA\pma.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\problem.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\sabase.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\solution.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\solutionsset.h"
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tbnode.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tbtree.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\ParetoGA\tindexedpoint.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tlistset.h"
# End Source File
# Begin Source File

SOURCE=.\tmoscpsolution.h
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tmotools.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tquadnode.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tquadtree.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\Common\trandomgenerator.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\weights.h"
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# Begin Source File

SOURCE=.\temp.txt
# End Source File
# End Target
# End Project

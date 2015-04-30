# Microsoft Developer Studio Project File - Name="NewProblemTemplate" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=NewProblemTemplate - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "NewProblemTemplate.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "NewProblemTemplate.mak" CFG="NewProblemTemplate - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NewProblemTemplate - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "NewProblemTemplate - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NewProblemTemplate - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x415 /d "NDEBUG"
# ADD RSC /l 0x415 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386

!ELSEIF  "$(CFG)" == "NewProblemTemplate - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "../MOMHLib++/MO" /I "../MOMHLib++/HGA" /I "../MOMHLib++/SA" /I "../MOMHLib++/MOMSLS" /I "../MOMHLib++/common" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x415 /d "_DEBUG"
# ADD RSC /l 0x415 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "NewProblemTemplate - Win32 Release"
# Name "NewProblemTemplate - Win32 Debug"
# Begin Source File

SOURCE="..\MOMHLib++\MO\global.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\Global.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momethod.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\MOMethod.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momhconstrainedsolution.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momhconstrainedsolution.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\momhsolution.h"
# End Source File
# Begin Source File

SOURCE=.\newproblem.cpp
# End Source File
# Begin Source File

SOURCE=.\newproblem.h
# End Source File
# Begin Source File

SOURCE=.\newproblemmain.cpp
# End Source File
# Begin Source File

SOURCE=.\newsolution.cpp
# End Source File
# Begin Source File

SOURCE=.\newsolution.h
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\nondominatedset.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\NondominatedSet.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\PROBLEM.H"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\sabase.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\SA\sabase.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\solution.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\SOLUTION.H"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\solutionsset.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\SolutionsSet.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tbnode.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tbnode.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tbtree.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tbtree.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\ParetoGA\tindexedpoint.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\ParetoGA\tindexedpoint.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tlistset.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tlistset.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tmotools.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tmotools.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tquadnode.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tquadnode.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tquadtree.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\tquadtree.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\Common\trandomgenerator.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\Common\trandomgenerator.h"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\weights.cpp"
# End Source File
# Begin Source File

SOURCE="..\MOMHLib++\MO\WEIGHTS.H"
# End Source File
# End Target
# End Project

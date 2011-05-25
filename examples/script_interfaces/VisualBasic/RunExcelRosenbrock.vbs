REM VisualBasic example of computing Rosenbrock in Excel

REM Create a shell object for filesystem commands
Set wshShell = WScript.CreateObject("WScript.shell")
mainDirectory = wshShell.CurrentDirectory

REM Open the excel workbook and select Sheet1
Set ExcelApp = CreateObject("Excel.Application")
Set ExcelWB = ExcelApp.Workbooks.open(mainDirectory & "\Rosenbrock.xls")
Set ExcelSheet = ExcelWB.WorkSheets("Sheet1")

REM get DAKOTA parameters from params.in
Set filesys = CreateObject("Scripting.FileSystemObject")
Set paramsFile = filesys.OpenTextFile("params.in", 1)

REM discard first line
strNextLine = paramsFile.Readline

REM extract x1 and x2
strNextLine = paramsFile.Readline
arrItemList = Split(LTrim(strNextLine))
x1 = arrItemList(0)
strNextLine = paramsFile.Readline
arrItemList = Split(LTrim(strNextLine))
x2 = arrItemList(0)
paramsFile.close()

WScript.echo("Setting parameters " & x1 & " and " & x2 & " in Excel")

REM set DAKOTA parameters in Excel
ExcelSheet.Cells(2,2) = x1
ExcelSheet.Cells(3,2) = x2

REM calculate rosenbrock function
ExcelSheet.Calculate()

REM extract results for DAKOTA
Set f = ExcelSheet.Cells(4,2)

REM return result to DAKOTA
Set resultsFile = filesys.OpenTextFile("results.tmp", 2, True)
strNextLine = resultsFile.Writeline(f & " f")
resultsFile.close()
filesys.MoveFile "results.tmp" , "results.out"

WScript.echo("Excel returned Rosenbrock f = " & f)

REM Cleanup activities
REM ExcelApp.Workbooks(1).Save
ExcelWB.Close(false)
ExcelApp.Quit
Set ExcelApp = Nothing

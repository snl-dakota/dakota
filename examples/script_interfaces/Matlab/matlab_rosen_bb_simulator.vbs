Dim mlobj

Set mlobj = CreateObject("Matlab.Application")

' Change to the directory where the parameters file will be written by Dakota, and 
' where Dakota expects to find the responses file after completion of this script
cd_command = "cd('c:\matlab_blackbox')"
mlobj.Execute(cd_command)

'Get the command line arguments passed by DAKOTA, so we work on the correct files
paramsfile  = WScript.Arguments.Item(0)
resultsfile = WScript.Arguments.Item(1)

Dim matlab_command
matlab_command = "matlab_rosen_wrapper('" & paramsfile & "', '" & resultsfile & "')"

Dim result
result = mlobj.Execute(matlab_command)
WScript.Echo "Matlab returned: " & result

'TODO: capture and act on any error from Matlab

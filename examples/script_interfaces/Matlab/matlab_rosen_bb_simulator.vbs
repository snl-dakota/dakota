Dim mlobj
Dim result

Set mlobj = CreateObject("Matlab.Application")

'may be necessary to change to absolute directory depending on Matlab behavior
mlobj.Execute("cd c:\matlab_blackbox")

'Get the command line arguments passed by DAKOTA, so we work on the correct files
paramsfile  = WScript.Arguments.Item(0)
resultsfile = WScript.Arguments.Item(1)

Dim matlab_command
matlab_command = "matlab_rosen_wrapper('" & paramsfile & "', '" & resultsfile & "')"

result = mlobj.Execute(matlab_command)
WScript.Echo "Matlab returned: " & result

'TODO: capture and act on any error from Matlab

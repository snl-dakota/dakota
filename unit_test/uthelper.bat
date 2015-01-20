@ECHO OFF
REM DOS conversion of usage-message.sh

REM A hack, but should meet the unit_test need.. AND now a .bat equivalent
REM : ${0?"Usage: $0 [ARGUMENT]"}

IF !%1==! GOTO SILENTEX
ECHO "These two lines echo only if DBG command-line parameter given."
ECHO "Unit test helper debug file written."
ECHO "Verify command-line parameter:  %0 %1" > ut_helper.out 2>&1

REM Debug usage of unit test helper script - allow file to persist
REM ERASE ut_helper.out
GOTO EXIT0

:SILENTEX
ECHO "Verify relative path to driver: %0" > ut_helper.out 2>&1
ECHO "Verify non-existant command-line parameter: %1" >> ut_helper.out 2>&1

REM Basic usage of unit test helper script - clean up
ERASE ut_helper.out

:EXIT0


REM Windows batch script (dakota will call to execute analysis code)
REM dakota will execute this script as
REM   dakota_textbook.bat params.in results.out
REM so %1 will be the parameters file and
REM    %2 will be the results file to return to dakota

REM mock pre-processing
copy %1 tb.in

REM run binary
echo "Running text_book.exe binary"
text_book.exe tb.in tb.out
del tb.in

REM mock post-processing
move tb.out %2


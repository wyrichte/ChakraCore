@echo off
setlocal
set _PERFJC=%1
set _OPTION=%2 %3 %4 %5 %6 %7 %8 %9 -bvt
set _TESTDIR=%~dp0\gcperf\
IF "%_PERFJC%" == "" (
    echo ERROR: Missing JC binary parameter
    exit /B 1
)
call :Runtest gc-mark.js
call :Runtest gc-marklib.js
call :Runtest gc-markleaf.js
call :Runtest gc-marksparsearray.js
call :Runtest gc-markproperty.js
call :Runtest gc-sweep.js
call :Runtest gc-sweep2.js
call :Runtest gc-weakref.js
call :Runtest gc-deeptypepath.js
call :Runtest gc-steady.js

endlocal
exit /B 0

:Runtest
echo %1
%_PERFJC% %_OPTION% %_TESTDIR%%1
exit /B 0


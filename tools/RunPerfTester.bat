@echo off

if DEFINED _BuildType if "%_BuildType%" neq "fre" (
    echo Performance tests should be run on fre builds.
    goto :eof
)

call robocopy "\\chakrafs\fs\Tools\vso\PerfTester" "%~dp0PerfTester" /e /xo /ndl /njh /njs

setlocal EnableDelayedExpansion
set Path=%Path%;%~dp0PerfTester
start PerfTester.exe %*
endlocal
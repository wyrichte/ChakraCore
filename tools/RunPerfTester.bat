@echo off

if "%_BuildType%" neq "fre" (
    echo Performance tests should be run on fre builds.
    goto :eof
)

call robocopy "\\bptstorage3\Users\PerfTester" "%~dp0PerfTester" /e /xo /ndl /njh /njs

setlocal EnableDelayedExpansion
set Path=%Path%;%~dp0PerfTester
start PerfTester.exe %*
endlocal

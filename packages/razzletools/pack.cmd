@echo off
setlocal

set NUGET_BASE_PATH=%SDXROOT%

call ..\create_package.cmd

endlocal

@echo off
setlocal

set NUGET_BASE_PATH=%PUBLIC_ROOT%

call ..\create_package.cmd

endlocal

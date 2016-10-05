@echo off
setlocal

set NUGET_PACKAGE_NAME=SDPublics
set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_BASE_PATH=%PUBLIC_ROOT%
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%

call ..\create_package.cmd

endlocal

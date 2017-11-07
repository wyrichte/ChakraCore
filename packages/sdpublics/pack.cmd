@echo off
setlocal

set NUGET_PACKAGE_NAME=SDPublics
set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_BASE_PATH=%OSBuildRoot%
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%

call %PACKAGE_PATH%\create_package.cmd

endlocal

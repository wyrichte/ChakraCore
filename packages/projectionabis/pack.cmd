@echo off
setlocal

set PACKAGE_PATH=%SDXROOT%\inetcore\jscript\packages
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\projectionabis
set NUGET_BASE_PATH=%NUGET_PACKAGE_SOURCE_DIR%\stage

call %PACKAGE_PATH%\create_package.cmd

endlocal

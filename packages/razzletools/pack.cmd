@echo off
setlocal

set NUGET_PACKAGE_NAME=RazzleTools
set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_BASE_PATH=%SDXROOT%
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%
pushd %~dp0
call copyPgortBin.bat %SDXROOT%\tools "x86,amd64,arm" %SDXROOT%\tools
rem These binaries gets populated when running sdx root
call robocopy /e %OSBuildUtilitiesRoot%\TAEF\ %SDXROOT%\tools\ /NFL /NDL /NJH /NJS /nc /ns /np

call ..\create_package.cmd
popd
endlocal

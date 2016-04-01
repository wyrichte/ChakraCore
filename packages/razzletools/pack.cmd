REM @echo off
setlocal

set NUGET_BASE_PATH=%SDXROOT%
pushd %~dp0
call copyPgortBin.bat %SDXROOT%\tools "x86,amd64,arm" %SDXROOT%\tools
call ..\create_package.cmd
popd
endlocal

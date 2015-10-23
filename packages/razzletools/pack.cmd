@echo off
setlocal

set NUGET_BASE_PATH=%SDXROOT%

call ..\..\tools\copyPgortBin.bat both %SDXROOT%\tools "" win32 %SDXROOT%\tools\x86
call ..\..\tools\copyPgortBin.bat both %SDXROOT%\tools "" x64 %SDXROOT%\tools\amd64
call ..\create_package.cmd

endlocal

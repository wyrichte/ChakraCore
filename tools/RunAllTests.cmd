::::::::::::::::::::::::::::::::::::::::::::::::
::
:: RunAllTests.cmd
::
:: Simple short-cut method of calling runjs.bat runConsoleUnitTests.
::
:: Executes unit tests for the current build type and architecture, optionally 
:: passing arguments to RunAllRLTests.cmd
::
::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
setlocal

set args=%*
set InDRT=false
set InRazzle=true
set addPath=%_nttree%\jscript
set JSToolsDir=%SDXROOT%\onecoreuap\inetcore\jscript\tools
set path=%path%;%addPath%
set Timeout=1800

pushd %JSToolsDir%
set _Error=0

call runjs.bat runConsoleUnitTests %_BuildType% %_BuildArch% %Timeout% "%args%" %InRazzle% %InDRT%
popd

if not errorlevel 0 (echo ERROR DETECTED & set _Error=1)
exit /b %_Error%

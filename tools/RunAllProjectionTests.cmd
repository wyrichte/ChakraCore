@echo off
setlocal

if "%1" == "-?" (
  echo Usage:
  echo  RunAllProjectionTests.cmd [-?] [-force]
  echo.
  echo. -force: will force unregister/re-register of ABIs
  exit /b 1
)

set %_Error%=0

set _TestsDirectory=
set path=%path%;%sdxroot%\onecoreuap\inetcore\jscript\tools;%_nttree%\jscript
set setupCmd=%sdxroot%\onecoreuap\inetcore\jscript\projectionTests\Tests\setup.cmd
set runCmd=%sdxroot%\onecoreuap\inetcore\jscript\projectionTests\Tests\runalltests.cmd -snapTests -logverbose %*

echo :: Running standalone Projection UnitTests:

pushd %sdxroot%\onecoreuap\inetcore\jscript\projectionTests\Tests

call runjs setupWindowsGlobalization

echo %setupCmd%
call %setupCmd% 2>&1

echo %runCmd%
call %runCmd% 2>&1

if not errorlevel 0 (echo ERROR DETECTED & set _Error=1)

popd

endlocal
exit /b %_Error%
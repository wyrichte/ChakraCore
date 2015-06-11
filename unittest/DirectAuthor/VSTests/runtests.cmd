@echo off 
setlocal

del /s /q Logs\* >nul 2>&1
rd /s /q Logs >nul 2>&1
md Logs

set devenvLogFile=Logs\devenv.log
set mstestLogFile=Logs\mstest.log
if "%msDevEnv%" equ "" set msDevEnv=%SDXROOT%\tools\winide\Rascal\Common7\IDE\devenv.exe
set devenvCommand="%msDevEnv%"
set testSolutionPath=%SDXROOT%\inetcore\jscript\unittest\DirectAuthor\VSTests\DirectAuthorTests\DirectAuthorTests.csproj
if "%msTestCommand%" equ "" set msTestCommand=%SDXROOT%\tools\winide\Rascal\Common7\IDE\mstest.exe
set testRunSettings=%SDXROOT%\inetcore\jscript\unittest\DirectAuthor\VSTests\VSTests.testsettings
set testRunMetaData=%SDXROOT%\inetcore\jscript\unittest\DirectAuthor\VSTests\VSTests.vsmdi
set buildFlavor=Debug

IF "%_BuildType%" == "fre" (
  set buildFlavor=Release
)

set build=true
set log=true

:loop
IF "%1" == "/b-" (
  shift
  set build=false
  goto loop
) ELSE IF "%1" == "/l-" (
  shift
  set log=false
  goto loop
)

if "%build%"=="false" ( goto MSTEST )
:Build
echo.
echo ############# Starting Test Binary build #############
echo    devenv location : %devenvCommand% 
echo    Test Solution   : %testSolutionPath% 
echo    Build Flavor    : %buildFlavor% 
echo    Log File        : %devenvLogFile% 
echo.

call  %devenvCommand% /Rebuild "%buildFlavor%|Any CPU" "%testSolutionPath%" /out %devenvLogFile%
set DevEnvSuccessPattern="========== Rebuild All: 3 succeeded, 0 failed, 0 skipped =========="

findstr /r /c:%DevEnvSuccessPattern% %devenvLogFile%
If %ERRORLEVEL% NEQ 0 ( 
  REM The string was NOT found... Build failed
  echo Build Failed. 
  goto :Error
) ELSE (
  goto :MSTEST
)

:MSTEST
echo.
echo ############# Starting Test Execution  #############
echo    MSTest location : %msTestCommand% 
echo    Test Settings   : %testRunSettings% 
echo    test Meta Data  : %testRunMetaData% 
echo    Log File        : %mstestLogFile% 
echo.

echo Running: "%msTestCommand%" "/testlist:Check-in tests" "/testsettings:%testRunSettings%" "/testmetadata:%testRunMetaData%"
if "%log%"=="true" (
  call "%msTestCommand%" "/testlist:Check-in tests" "/testsettings:%testRunSettings%" "/testmetadata:%testRunMetaData%" > %mstestLogFile% 2>&1
) ELSE (
  call "%msTestCommand%" "/testlist:Check-in tests" "/testsettings:%testRunSettings%" "/testmetadata:%testRunMetaData%"
)

set MSTestSuccessPattern="Test Run Completed\.$"
findstr /r /c:%MSTestSuccessPattern% %mstestLogFile%
If %ERRORLEVEL% NEQ 0 (
  REM The string was NOT found... test run failed
  goto :Error
)

echo.
echo Test Run Succeeded.

REM Delete any left over files if the test was successful
del /s /q TestResults\* >nul 2>&1
rd /s /q TestResults >nul 2>&1

goto :DONE

:ERROR
echo.
echo Test Run Failed.
goto :DONE

:DONE
endlocal



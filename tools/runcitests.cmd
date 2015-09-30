:: ============================================================================
::
:: runcitests.cmd
::
:: Runs tests for continuous integration.  This script is called from the VSO
:: build and it runs all tests for x86 and x64, debug and test build configs.
:: Logs are copied to build drop.
::
:: Do not use this script to run all tests on your dev box.
::   - It will delete all your existing test logs
::   - It does not run the various flavors of the tests in parallel (though
::     this is not currently possible anyway because rl stages logs in a
::     common directory)
::   - It does nothing to provide useful output when there are failures, e.g.
::     they can be buried under thousands of lines of output from further
::     tests run.
::   - It cannot be cancelled without risk of polluting your command prompt
::     environment with environment variables that will make further calls to
::     runtests.cmd behave unexpectedly.
::   - It will copy the logs to a folder named \testlogs in the directory you
::     started this script from.
::
:: ============================================================================
@echo off
setlocal

set _RootDir=%~dp0..
set _ToolsDir=%~dp0
set _StagingDir=%TF_BUILD_BINARIESDIRECTORY%
set _DropRootDir=%TF_BUILD_DROPLOCATION%
set _HadFailures=0

:: ============================================================================
:: Main script
:: ============================================================================
:main

  if not "%TF_BUILD%" == "True" (
    echo This script must be run under a TF Build Agent environment
    exit /b 2
  )

  call :doSilent rd /s/q %_RootDir%\core\test\logs
  call :doSilent rd /s/q %_RootDir%\unittest\logs

  call :runTests core x86debug
  call :runTests core x86test
  call :runTests core x64debug
  call :runTests core x64test
  call :runTests unit x86debug
  call :runTests unit x86test
  call :runTests unit x64debug
  call :runTests unit x64test
  call :runJsRTTests x86debug
  call :runJsRTTests x86test
  call :runJsRTTests x64debug
  call :runJsRTTests x64test

  call :summarizeLogs
  call :copyLogsToDrop

  echo.
  if "%_HadFailures%" == "1" (
    echo -- runcitests.cmd ^>^> Tests failed! 1>&2
  ) else (
    echo -- runcitests.cmd ^>^> Tests passed!
  )
  echo -- runcitests.cmd ^>^> Logs at %_DropRootDir%\testlogs

  exit /b %_HadFailures%

:: ============================================================================
:: Run one test suite against one build config and record if there were errors
:: ============================================================================
:runTests

  call :do %_ToolsDir%run%1tests.cmd -%2 -quiet -cleanupall -binDir %_StagingDir%\bin

  if ERRORLEVEL 1 set _HadFailures=1

  goto :eof

:: ============================================================================
:: Run jsrt test suite against one build config and record if there were errors
:: ============================================================================
:runJsRTTests

  call :do %_ToolsDir%runjsrttests.cmd -%1 -binDir %_StagingDir%\bin

  if ERRORLEVEL 1 set _HadFailures=1

  goto :eof

:: ============================================================================
:: Copy all result logs to the drop share
:: ============================================================================
:copyLogsToDrop

  :: /S Copy all non-empty dirs
  :: /Y Do not prompt for overwriting destination files
  :: /C Continue copying if there are errors
  :: /I Assume destination is a directory if it does not exist

  call :do xcopy %_RootDir%\core\test\logs %_StagingDir%\testlogs\coretest /S /Y /C /I
  call :do xcopy %_RootDir%\unittest\logs %_StagingDir%\testlogs\unittest /S /Y /C /I
  call :do move %_StagingDir%\testlogs\coretest\summary.coretest.log %_StagingDir%\testlogs
  call :do move %_StagingDir%\testlogs\unittest\summary.unittest.log %_StagingDir%\testlogs

  goto :eof

:: ============================================================================
:: Summarize the logs into a listing of only the failures
:: ============================================================================
:summarizeLogs

  pushd %_RootDir%\core\test\logs
  findstr /sp failed rl.results.log > summary.coretest.log
  type summary.coretest.log 1>&2
  popd

  pushd %_RootDir%\unittest\logs
  findstr /sp failed rl.results.log > summary.unittest.log
  type summary.unittest.log 1>&2
  popd

:: ============================================================================
:: Echo a command line before executing it
:: ============================================================================
:do

  echo -- runcitests.cmd ^>^> %*
  cmd /s /c "%*"

  goto :eof

:: ============================================================================
:: Echo a command line before executing it and redirect the command's output
:: to nul
:: ============================================================================
:doSilent

  echo -- runcitests.cmd ^>^> %* ^> nul 2^>^&1
  cmd /s /c "%* > nul 2>&1"

  goto :eof

:: ============================================================================
::
:: runjsrttests.cmd
::
:: Runs JsRT native unittests.
::
:: ============================================================================

@echo off
setlocal

goto :main

:: ============================================================================
:: Print usage
:: ============================================================================
:printUsage

  echo runjsrttests.cmd -x86^|-x64 -debug^|-test
  echo.
  echo Required switches:
  echo.
  echo   Specify architecture of jsrt binaries (required):
  echo.
  echo   -x86           Build arch of binaries is x86
  echo   -x64           Build arch of binaries is x64
  echo.
  echo   Specify type of jsrt binaries (required):
  echo.
  echo   -debug         Build type of binaries is debug
  echo   -test          Build type of binaries is test
  echo.
  echo   Shorthand combinations can be used, e.g. -x64debug
  echo.

  goto :eof

:: ============================================================================
:: Print how to get help
:: ============================================================================
:printGetHelp

  echo For help use runjsrttests.cmd -?

  goto :eof

:: ============================================================================
:: Main script
:: ============================================================================
:main

  call :initVars
  call :parseArgs %*

  if not "%fShowUsage%" == "" (
    call :printUsage
    goto :eof
  )

  call :validateArgs

  if not "%fShowGetHelp%" == "" (
    call :printGetHelp
    goto :eof
  )

  call :configureVars

  call :copyScriptsAndBinaries

  call :runtest

  call :cleanup

  exit /b %_HadFailures%

:: ============================================================================
:: Initialize batch script variables to defaults
:: ============================================================================
:initVars

  set _HadFailures=0
  set _RootDir=%~dp0..
  set _BinDirBase=%_RootDir%\Build\VcBuild\Bin
  set _BinDir=
  set _TestTempDir=
  set _BuildArch=
  set _BuildType=
  REM Auto read the RazzleTools package version from the packages.config
  for /f tokens^=4^ delims^=^" %%i in ('findstr /R "RazzleTools.*[0-9]\.[0-9]\.[0-9]" %_RootDir%\packages.config') do set _RazzleTools=RazzleTools.%%i
  goto :eof

:: ============================================================================
:: Parse the user arguments into environment variables
:: ============================================================================
:parseArgs

  :NextArgument

  if "%1" == "-?" set fShowUsage=1& goto :ArgOk
  if "%1" == "/?" set fShowUsage=1& goto :ArgOk

  if /i "%1" == "-x86"              set _BuildArch=x86&                                         goto :ArgOk
  if /i "%1" == "-x64"              set _BuildArch=x64&                                         goto :ArgOk
  if /i "%1" == "-debug"            set _BuildType=debug&                                       goto :ArgOk
  if /i "%1" == "-test"             set _BuildType=test&                                        goto :ArgOk

  if /i "%1" == "-x86debug"         set _BuildArch=x86&set _BuildType=debug&                    goto :ArgOk
  if /i "%1" == "-x64debug"         set _BuildArch=x64&set _BuildType=debug&                    goto :ArgOk
  if /i "%1" == "-x86test"          set _BuildArch=x86&set _BuildType=test&                     goto :ArgOk
  if /i "%1" == "-x64test"          set _BuildArch=x64&set _BuildType=test&                     goto :ArgOk

  if /i "%1" == "-binDir"           set _BinDirBase=%~f2&                                      goto :ArgOkShift2

  if not "%1" == "" echo -- runjsrttests.cmd ^>^> Unknown argument: %1 & set fShowGetHelp=1

  goto :eof

  :ArgOkShift2
  shift

  :ArgOk
  shift

  goto :NextArgument

:: ============================================================================
:: Validate that required arguments were specified
:: ============================================================================
:validateArgs

  if "%_BuildArch%" == "" (
    echo -- runjsrttests.cmd ^>^> Error missing required build architecture or build type switch
    set fShowGetHelp=1
    goto :eof
  )

  if "%_BuildType%" == "" (
    echo -- runjsrttests.cmd ^>^> Error missing required build architecture or build type switch
    set fShowGetHelp=1
  )

  goto :eof

:: ============================================================================
:: Configure the script variables and environment based on parsed arguments
:: ============================================================================
:configureVars

  set _BinDir=%_BinDirBase%\%_BuildArch%_%_BuildType%\
  rem Use %_BinDir% as the root for %_TestTempDir% to ensure that running jsrt
  rem tests from multiple repos simultaneously do not clobber each other.
  rem This means we need to delete the temp directory afterwards to clean up.
  set _TestTempDir=%_BinDir%\jsrttest\
  if not exist %_TestTempDir% mkdir %_TestTempDir%

  echo -- runjsrttests.cmd ^>^> #### BinDir: %_BinDir%
  echo -- runjsrttests.cmd ^>^> #### TestTempDir: %_TestTempDir%

  goto :eof

:: ============================================================================
:: Copying javascript files from source location to temp test location and test
:: binaries from binary location to test temp location.
:: ============================================================================
:copyScriptsAndBinaries

  echo -- runjsrttests.cmd ^>^> copying scripts from '%_RootDir%\unittest\jsrt\scripts' to '%_TestTempDir%'
  copy /y %_RootDir%\unittest\jsrt\scripts\*.js %_TestTempDir%

  echo -- runjsrttests.cmd ^>^> copying winmd files to '%_TestTempDir%'
  copy /y %_BinDir%*.winmd %_TestTempDir%

  copy /y %_BinDir%Chakra.dll %_TestTempDir%
  copy /y %_BinDir%UnitTest.JsRT.API.dll %_TestTempDir%
  copy /y %_BinDir%UnitTest.JsRT.ComProjection.dll %_TestTempDir%
  copy /y %_BinDir%UnitTest.JsRT.RentalThreading.dll %_TestTempDir%
  copy /y %_BinDir%UnitTest.JsRT.WinRT.dll %_TestTempDir%
  copy /y %_BinDir%WinRt2TsTest.dll %_TestTempDir%

  goto :eof

:: ============================================================================
:: Running tests using te.exe
:: ============================================================================
:runtest

  set _AllTestBins=
  set _AllTestBins=%_AllTestBins% %_TestTempDir%UnitTest.JsRT.API.dll
  set _AllTestBins=%_AllTestBins% %_TestTempDir%UnitTest.JsRT.ComProjection.dll
  set _AllTestBins=%_AllTestBins% %_TestTempDir%UnitTest.JsRT.RentalThreading.dll
  set _AllTestBins=%_AllTestBins% %_TestTempDir%UnitTest.JsRT.WinRT.dll
  set _AllTestBins=%_AllTestBins% %_TestTempDir%WinRt2TsTest.dll

  set _RazzleBuildArch=%_BuildArch%
  if "%_BuildArch%" == "x64" set _RazzleBuildArch=amd64

  set _TECommand=%_RootDir%\External\%_RazzleTools%\%_RazzleBuildArch%\te
  call :do %_TECommand% %_AllTestBins%  /select:"not(@Disabled='Yes')" /unicodeoutput:false /parallel:1 /logOutput:lowest

  if ERRORLEVEL 1 set _HadFailures=1

  goto :eof

:: Clean up after running tests
:cleanup

  call :do rd /s/q %_TestTempDir%

  goto :eof

:: ============================================================================
:: Echo a command line before executing it
:: ============================================================================
:do

  echo -- runjsrttests.cmd ^>^> %*
  cmd /s /c "%*"

  goto :eof

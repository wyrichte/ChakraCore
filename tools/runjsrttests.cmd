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

  call :copyScripts

  call :runtest

  exit /b %_HadFailures%

:: ============================================================================
:: Initialize batch script variables to defaults
:: ============================================================================
:initVars

  set _HadFailures=0
  set _RootDir=%~dp0..
  set _BinDirBase=%_RootDir%\Build\VcBuild\Bin
  set _BinDir=
  set _BuildArch=
  set _BuildType=
  set _RazzleTools=RazzleTools.1.0.8
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

  if not "%1" == "" echo Unknown argument: %1 & set fShowGetHelp=1

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
    echo Error missing required build architecture or build type switch
    set fShowGetHelp=1
    goto :eof
  )

  if "%_BuildType%" == "" (
    echo Error missing required build architecture or build type switch
    set fShowGetHelp=1
  )

  goto :eof

:: ============================================================================
:: Copying javascript files from source location to binary location
:: ============================================================================
:copyScripts

  set _BinDir=%_BinDirBase%\%_BuildArch%_%_BuildType%\
  echo copying scripts from : %_RootDir%\unittest\jsrt\scripts\ to : %_BinDir%
  copy /y %_RootDir%\unittest\jsrt\scripts\*.js %_BinDir%

  goto :eof

:: ============================================================================
:: Running tests using te.exe
:: ============================================================================
:runtest

  set _AllTestBins=
  set _AllTestBins=%_AllTestBins% %_BinDir%UnitTest.JsRT.API.dll
  set _AllTestBins=%_AllTestBins% %_BinDir%UnitTest.JsRT.ComProjection.dll
  set _AllTestBins=%_AllTestBins% %_BinDir%UnitTest.JsRT.MemoryPolicy.dll
  set _AllTestBins=%_AllTestBins% %_BinDir%UnitTest.JsRT.RentalThreading.dll
  set _AllTestBins=%_AllTestBins% %_BinDir%UnitTest.JsRT.ThreadService.dll
  set _AllTestBins=%_AllTestBins% %_BinDir%UnitTest.JsRT.WinRT.dll

  set _RazzleBuildArch=%_BuildArch%
  if "%_BuildArch%" == "x64" set _RazzleBuildArch=amd64

  set _TECommand=%_RootDir%\External\%_RazzleTools%\%_RazzleBuildArch%\te
  call :do %_TECommand% %_AllTestBins%  /select:"not(@Disabled='Yes')" /unicodeoutput:false /parallel:1 /logOutput:lowest

  if ERRORLEVEL 1 set _HadFailures=1

  goto :eof

:: ============================================================================
:: Echo a command line before executing it
:: ============================================================================
:do

  echo -- runjsrttests.cmd ^>^> %*
  cmd /s /c "%*"

  goto :eof

@echo off

setlocal

set arch=%1
set flavor=%2
set doCore=%3
set builderror=
set PogoConfig=True

if "%arch%"=="" (
  goto:usage
)
if "%flavor%"=="" (
  goto:usage
)
for %%i in ("%~dp0..") do set "ChakraRoot=%%~fi"

call %ChakraRoot%\core\Build\scripts\add_msbuild_path.cmd
set _ChakraSolution=%ChakraRoot%\Build\Chakra.Full.sln
if "%doCore%" EQU "core" (
    set _ChakraSolution=%ChakraRoot%\core\Build\Chakra.Core.sln
)
set BIN_PATH=%ChakraRoot%\Build\VcBuild\bin\%arch%_%flavor%_pogo
if "%doCore%" EQU "core" (
    set BIN_PATH=%ChakraRoot%\core\Build\VcBuild\bin\%arch%_%flavor%_pogo
)
if "%TF_BUILD_BUILDDIRECTORY%" NEQ "" (
  set BIN_PATH=%TF_BUILD_BUILDDIRECTORY%\bin\bin\%arch%_%flavor%_pogo
) else if "%OutBaseDir%" NEQ "" (
  set BIN_PATH=%OutBaseDir%\Chakra.Full\bin\%arch%_%flavor%_pogo
  if "%doCore%" EQU "core" (
    set BIN_PATH=%OutBaseDir%\Chakra.Core\bin\%arch%_%flavor%_pogo
  )
)

call %ChakraRoot%\core\Build\scripts\pgo\pre_pgi.cmd %arch% %flavor% "%BIN_PATH%"
if "%errorlevel%" NEQ "0" (
    exit /b %errorlevel%
)
call :build
if /I "%builderror%" NEQ "" (
  echo %builderror%: An error occurred in the build instrumentation
  endlocal
  exit /b %builderror%
)
call %ChakraRoot%\core\Build\scripts\pgo\post_pgi.cmd
if "%errorlevel%" NEQ "0" (
    exit /b %errorlevel%
)

IF /I "%arch%" NEQ "x86" (
  IF /I "%arch%" NEQ "x64" (
    REM We can only train x86 and x64 on the build machine.
    endlocal
    goto:eof
  )
)

set pgi_bin=jshost.exe
if "%doCore%" EQU "core" (
    set pgi_bin=ch.exe
)
 REM Do training
call powershell %ChakraRoot%\core\Build\Scripts\pgo\pogo_training.ps1 -scenarios %ChakraRoot%\core\test\benchmarks\SunSpider,%ChakraRoot%\core\test\benchmarks\Kraken,%ChakraRoot%\core\test\benchmarks\Octane -binary %BIN_PATH%\%pgi_bin% -arch %arch% -flavor %flavor%
if "%errorlevel%" NEQ "0" (
    exit /b %errorlevel%
)
call %ChakraRoot%\core\Build\scripts\pgo\pre_pgo.cmd
if "%errorlevel%" NEQ "0" (
    exit /b %errorlevel%
)
call :build
if /I "%builderror%" NEQ "" (
  echo %builderror%: An error occurred in the build optimization
  endlocal
  exit /b %builderror%
)
call %ChakraRoot%\core\Build\scripts\pgo\post_pgo.cmd %BIN_PATH%
if "%errorlevel%" NEQ "0" (
    exit /b %errorlevel%
)

endlocal
goto:eof

:build
  set logsDirectory=%BIN_PATH%\..\..\buildlogs
  IF NOT EXIST %logsDirectory% (
    MD %logsDirectory%
  )
  set logFileName=%logsDirectory%\build_%arch%%flavor%%POGO_TYPE%
  set _LoggingParams=/fl1 /flp1:logfile=%logFileName%.log;verbosity=normal /fl2 /flp2:logfile=%logFileName%.err;errorsonly /fl3 /flp3:logfile=%logFileName%.wrn;warningsonly
  set Configuration=jshost-%flavor%
  if "%doCore%" EQU "core" (
    set Configuration=%flavor%
  )
  echo msbuild /m /nr:false /verbosity:minimal /p:Configuration=%Configuration% /p:Platform=%arch% "%_ChakraSolution%" %_LoggingParams%
  call msbuild /m /nr:false /verbosity:minimal /p:Configuration=%Configuration% /p:Platform=%arch% "%_ChakraSolution%" %_LoggingParams%
  if "%errorlevel%" NEQ "0" (
    set builderror=%errorlevel%
  )
goto:eof

:usage
  echo Invalid/missing arguments
  echo.
  echo Usage: pogo.git.bat ^<arch^> ^<flavor^>
  echo   - arch  : Architecture you want to build pogo
  echo   - flavor: Flavor you want to build pogo

exit /b 1

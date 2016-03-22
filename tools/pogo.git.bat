rem @echo off

setlocal

set arch=%1
set flavor=%2
set builderror=

if "%arch%"=="" (
  goto:usage
)
if "%flavor%"=="" (
  goto:usage
)
for %%i in ("%~dp0..") do set "ChakraRoot=%%~fi"
call %ChakraRoot%\tools\GitScripts\add-msbuild-path.cmd
set _ChakraSolution=%ChakraRoot%\Build\Chakra.Full.sln
set BIN_PATH=%ChakraRoot%\Build\VcBuild\bin\%arch%_%flavor%_pogo
if "%TF_BUILD_BUILDDIRECTORY%" NEQ "" (
  set BIN_PATH=%TF_BUILD_BUILDDIRECTORY%\bin\bin\%arch%_%flavor%_pogo
) else if "%OutBaseDir%" NEQ "" (
  set BIN_PATH=%OutBaseDir%\Chakra.Full\bin\%arch%_%flavor%_pogo
)

if not exist %BIN_PATH% (
  md %BIN_PATH%
) else (
  if exist %BIN_PATH%\*.pgc ( del %BIN_PATH%\*.pgc )
)

 REM Build with pgo instrumentation
set POGO_TYPE=PGI

REM Temporary fix around pgo bug, todo:: check if still necessary once toolset is updated
set _LINK_=/cgthreads:1
call :build
set _LINK_=

if /I "%builderror%" NEQ "" (
  echo %builderror%: An error occurred in the build instrumentation
  endlocal
  exit /b %builderror%
)
set POGO_TYPE=

IF /I "%arch%" NEQ "x86" (
  IF /I "%arch%" NEQ "x64" (
    REM We can only train x86 and x64 on the build machine.
    endlocal
    goto:eof
  )
)

 REM Do training
for %%A in (%ChakraRoot%\core\test\benchmarks\SunSpider\*.js) do call %BIN_PATH%\jshost.exe %%A
for %%A in (%ChakraRoot%\core\test\benchmarks\Kraken\*.js) do call %BIN_PATH%\jshost.exe %%A
for %%A in (%ChakraRoot%\core\test\benchmarks\Octane\*.js) do call %BIN_PATH%\jshost.exe %%A

 REM Optimize build with pgo data
set POGO_TYPE=PGO
call :build
if /I "%builderror%" NEQ "" (
  echo %builderror%: An error occurred in the build optimization
  endlocal
  exit /b %builderror%
)
set POGO_TYPE=

 REM Clean binaries we no longer need
if exist %BIN_PATH%\*.pgc ( del %BIN_PATH%\*.pgc )
if exist %BIN_PATH%\*.pgd ( del %BIN_PATH%\*.pgd )
if exist %BIN_PATH%\pgort* ( del %BIN_PATH%\pgort* )

endlocal
goto:eof

:build
  set logFileName=%BIN_PATH%\build_%arch%%flavor%%POGO_TYPE%
  set _LoggingParams=/fl1 /flp1:logfile=%logFileName%.log;verbosity=normal /fl2 /flp2:logfile=%logFileName%.err;errorsonly /fl3 /flp3:logfile=%logFileName%.wrn;warningsonly
  echo msbuild /m /p:Configuration=jshost-%flavor% /p:Platform=%arch% "%_ChakraSolution%" %_LoggingParams%
  call msbuild /m /p:Configuration=jshost-%flavor% /p:Platform=%arch% "%_ChakraSolution%" %_LoggingParams%
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

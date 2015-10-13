@echo off

setlocal

if "%REPO_ROOT%"=="" (
  echo Run tools\GitScripts\init.cmd prior to running this script
  exit /b 1
)

if "%_BuildType%" NEQ "fre" (
  echo Can only build pogo on Release
  exit /b 1
)

 REM Build with pgo instrumentation
set POGO_TYPE=PGI
call %REPO_ROOT%\tools\GitScripts\build.cmd jshost
set POGO_TYPE=

set BIN_PATH=%REPO_ROOT%\Build\VcBuild\bin\%_BuildArch%_Pogo

del %BIN_PATH%\*.pgc

 REM Do training
for %%A in (%REPO_ROOT%\unittest\SunSpider1.0.2\*.js) do call %BIN_PATH%\jshost.exe %%A
for %%A in (%REPO_ROOT%\unittest\Kraken\*.js) do call %BIN_PATH%\jshost.exe %%A
call %BIN_PATH%\jshost.exe %REPO_ROOT%\unittest\Octane\octaneFull.js

set POGO_TYPE=PGO
call %REPO_ROOT%\tools\GitScripts\build.cmd jshost
set POGO_TYPE=

 REM Clean binaries we no longer need
del %BIN_PATH%\*.pgc
del %BIN_PATH%\pgort120.dll
del %BIN_PATH%\pgort120.pdb
del %BIN_PATH%\msvcr120.*

endlocal

@echo off

set _BuildArch=x86
set _BuildType=chk
for %%i in ("%~dp0..\..") do set "REPO_ROOT=%%~fi"
call %~dp0\add-msbuild-path.cmd

if "%PYTHON_PATH%" EQU "" (
   REM Hardcoded to the path that the enlistment script installs python to
   set PYTHON_PATH=C:\Python35
) else (
   echo Python located in %PYTHON_PATH%
)

:ContinueArgParse
if not "%1"=="" (
    if "%1"=="x86" (
        set _BuildArch=x86
    )
    if "%1"=="amd64" (
        set _BuildArch=x64
    )
    if "%1"=="x64" (
        set _BuildArch=x64
    )
    if "%1"=="arm64" (
        set _BuildArch=arm64
    )
    if "%1"=="arm" (
        set _BuildArch=arm
    )
    if "%1"=="fre" (
        set _BuildType=fre
    )
    if "%1"=="test" (
        set _BuildType=test
    )
    if "%1"=="chk" (
        set _BuildType=chk
    )
    shift
    goto :ContinueArgParse
)

REM Check for tools, setup path variable


:SkipMsBuildSetup
REM ========================================
REM Set up Git.exe
REM ========================================

where /q git.exe
IF "%ERRORLEVEL%" == "0" (
    goto :SkipGitSetup
)

set GIT_PATH=%ProgramFiles%\Git\cmd
if not exist "%GIT_PATH%\git.exe" (
    set GIT_PATH="%ProgramFiles(x86)%\Git\cmd"
)

if not exist %GIT_PATH%\git.exe (
    echo Can't find git.exe in %GIT_PATH%
) else (
    echo Verified Git
)

set PATH=%PATH%;%GIT_PATH%;
set GIT_PATH=


:SkipGitSetup

REM Bit of a hack but I can't think of a better way right now
REM to get git-remote-mshttps in path
set PATH=%PATH%;%ProgramFiles(x86)%\Git\libexec\git-core
set PATH=%PATH%;%REPO_ROOT%\tools\GitScripts

if not exist "%PYTHON_PATH%" (
   echo WARNING: Python not found in %PYTHON_PATH%
   echo Please run setup from \\chakrafs01\RepoTools\dep\python_installer.exe
   echo Install Python to C:\Python35
   echo Without this, Chakra git scripts will not work
   echo.
)

set PATH=%PATH%;%PYTHON_PATH%
set PATH=%PATH%;%PYTHON_PATH%\scripts

REM ============================================
REM Run initialization script from developer directory if one exists
REM ============================================
if "%DEV_DIR%" NEQ "" (
    if exist "%DEV_DIR%\init.cmd" (
        call %DEV_DIR%\init.cmd
    )
)

pushd %REPO_ROOT%

echo Environment initialized

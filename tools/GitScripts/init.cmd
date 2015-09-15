@echo off

set _BuildArch=x86
set _BuildType=chk

REM HACK: Surely someone has a nicer way to get the parent directory?
set CURRENT_DIR=%~dp0
pushd %CURRENT_DIR%
cd ..\..
set REPO_ROOT=%CD%
popd

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
    if "%1"=="chk" (
        set _BuildTypechk=
    )
    shift
    goto :ContinueArgParse
)

REM Check for tools, setup path variable


REM ========================================
REM Set up msbuild.exe
REM ========================================
where /q msbuild.exe 
IF "%ERRORLEVEL%" == "0" (
    goto :SkipMsBuildSetup
)

REM Try Dev14 first
set MSBUILD_VERSION=14.0
set MSBUILD_PATH=%ProgramFiles%\msbuild\%MSBUILD_VERSION%\Bin\x86

if not exist "%MSBUILD_PATH%\msbuild.exe" (
    set "MSBUILD_PATH=%ProgramFiles(x86)%\msbuild\%MSBUILD_VERSION%\Bin\amd64"
)

if exist "%MSBUILD_PATH%\msbuild.exe" (
    goto :MSBuildFound
)

echo Dev14 not found, trying Dev %MSBUILD_VERSION%
set MSBUILD_VERSION=12.0
set MSBUILD_PATH=%ProgramFiles%\msbuild\%MSBUILD_VERSION%\Bin\x86

if not exist "%MSBUILD_PATH%\msbuild.exe" (
    set "MSBUILD_PATH=%ProgramFiles(x86)%\msbuild\%MSBUILD_VERSION%\Bin\amd64"
)

if not exist "%MSBUILD_PATH%\msbuild.exe" (
    echo Can't find msbuild.exe in %MSBUILD_PATH%
    goto :SkipMsBuildSetup
) 

:MSBuildFound
echo MSBuild located at %MSBUILD_PATH%

set PATH=%MSBUILD_PATH%;%PATH%
set MSBUILD_PATH=

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

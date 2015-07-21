@echo off

set _BuildArch=x86
set _BuildType=chk

REM HACK: Surely someone has a nicer way to get the parent directory?
set CURRENT_DIR=%~dp0
pushd %CURRENT_DIR%
cd ..\..
set REPO_ROOT=%CD%
popd

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
REM TODO: Different MSBUILD version?
set MSBUILD_VERSION=12.0
set MSBUILD_PATH=%ProgramFiles%\msbuild\%MSBUILD_VERSION%\Bin\x86

if not exist "%MSBUILD_PATH%\msbuild.exe" (
    set MSBUILD_PATH="%ProgramFiles(x86)%\msbuild\%MSBUILD_VERSION%\Bin\amd64"
)

if not exist %MSBUILD_PATH%\msbuild.exe (
    echo Can't find msbuild.exe in %MSBUILD_PATH%
) else (
    echo Verified MSBuild
)

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

set PATH=%PATH%;%GIT_PATH%;%REPO_ROOT%\tools\GitScripts
set GIT_PATH=


:SkipGitSetup

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

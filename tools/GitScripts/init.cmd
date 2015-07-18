@echo off

set GIT_PATH=%ProgramFiles%\Git\cmd
if not exist "%GIT_PATH%\git.exe" (
    set GIT_PATH="%ProgramFiles(x86)%\Git\cmd"
)

set MSBUILD_VERSION=12.0
set _BuildArch=x86
set _BuildType=chk

:: HACK: Surely someone has a nicer way to get the parent directory?
set CURRENT_DIR=%~dp0
pushd %CURRENT_DIR%
cd ..\..
set REPO_ROOT=%CD%
popd

:ContinueArgParse
if not "%1"=="" (
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

    shift
    goto :ContinueArgParse
)

:: Check for tools, setup path variable
set MSBUILD_PATH=%ProgramFiles%\msbuild\%MSBUILD_VERSION%\Bin\x86

if not exist "%MSBUILD_PATH%\msbuild.exe" (
    set MSBUILD_PATH="%ProgramFiles(x86)%\msbuild\%MSBUILD_VERSION%\Bin\amd64"
)

if not exist %MSBUILD_PATH%\msbuild.exe (
    echo Can't find msbuild.exe in %MSBUILD_PATH%
) else (
    echo Verified MSBuild
)

if not exist %GIT_PATH%\git.exe (
    echo Can't find git.exe in %GIT_PATH%
) else (
    echo Verified Git
)

set PATH=%PATH%;%GIT_PATH%;%MSBUILD_PATH%;%REPO_ROOT%\tools\GitScripts

:: Run initialization script from developer directory if one exists
if "%DEV_DIR%" NEQ "" (
    if exist "%DEV_DIR%\init.cmd" (
        call %DEV_DIR%\init.cmd
    )
)

pushd %REPO_ROOT%

echo Environment initialized
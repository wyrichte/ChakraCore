@echo off
setlocal enableextensions

set JS_ROOT=%SDXROOT%\onecoreuap\inetcore\jscript

set BUILD_DEP_DIRS=^
%JS_ROOT%;^
%SDXROOT%\inetcore\onecoreuap\inetcore\jscript;

set NUGET_PACKAGE_OUT=%JS_ROOT%\packages\package_output
set NUGET_PACKAGE_DEPLOY=\\chakrafs\fs\Misc\NuGet

if EXIST %NUGET_PACKAGE_OUT% (
    echo Warning: Deleting everything in %NUGET_PACKAGE_OUT%
    if "%1" NEQ "/f" (
        pause
    )
    rmdir /q /s %NUGET_PACKAGE_OUT%
)
pushd %JS_ROOT%

for %%a in (x86 amd64 arm) do (
    for %%f in (chk fre) do (
        echo Building dependencies for %%a%%f..
        cmd /c %SDXROOT%\tools\razzle.cmd %%a%%f sharepublic no_oacr no_certcheck exec build -parent -c "%BUILD_DEP_DIRS%" 1>NUL
    )
)

for /f "usebackq" %%i in (`dir /a:d /b %JS_ROOT%\packages`) do (
    if exist %JS_ROOT%\packages\%%i\pack.cmd (
        echo Staging and packing %%i...
        pushd %JS_ROOT%\packages\%%i
        call pack.cmd
        popd
    )
)

endlocal

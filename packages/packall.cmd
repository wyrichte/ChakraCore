@echo off
setlocal enableextensions

set JS_ROOT=%SDXROOT%\onecoreuap\inetcore\jscript
set NUGET_PACKAGE_OUT=%JS_ROOT%\packages\package_output
set NUGET_PACKAGE_DEPLOY=\\chakrafs01\Nuget\packages

set BUILD_DEP_DIRS=^
%SDXROOT%\onecoreuap\inetcore\jscript\publish0;^
%SDXROOT%\onecoreuap\inetcore\jscript\publish;^
%SDXROOT%\onecoreuap\inetcore\manifests\inbox;^
%SDXROOT%\onecoreuap\inetcore\lib\devtb\dtbhost;^
%SDXROOT%\onecoreuap\inetcore\lib\ScriptProjectionHost\iel3_edge;^
%SDXROOT%\onecoreuap\inetcore\jscript\ProjectionTests;^
%SDXROOT%\onecoreuap\inetcore\jscript;

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
        cmd /c %SDXROOT%\tools\razzle.cmd %%a%%f sharepublic no_oacr no_certcheck exec build /czPh /dir "%BUILD_DEP_DIRS%" 1>NUL
    )
)

for /f "usebackq" %%i in (`dir /a:d /b %JS_ROOT%\packages`) do (
    if exist %JS_ROOT%\packages\%%i\pack.cmd (
        echo Packaging %%i
        pushd %JS_ROOT%\packages\%%i
        call pack.cmd
        popd
    )
)

endlocal

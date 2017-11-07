@echo off
setlocal enableextensions

set JS_ROOT=%SDXROOT%\onecoreuap\inetcore\jscript

set BUILD_DEP_DIRS=^
%SDXROOT%\onecoreuap\inetcore\jscript\publish0;^
%SDXROOT%\onecoreuap\inetcore\jscript\publish;^
%SDXROOT%\onecoreuap\inetcore\manifests\inbox;^
%SDXROOT%\onecoreuap\inetcore\lib\devtb\dtbhost;^
%SDXROOT%\onecoreuap\inetcore\lib\ScriptProjectionHost\iel3_edge;^
%SDXROOT%\onecoreuap\inetcore\jscript;^
%SDXROOT%\inetcore\onecoreuap\inetcore\jscript\ProjectionTests;

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

rem goto:doPack

for %%a in (x86 amd64 arm) do (
    for %%f in (chk fre) do (
        echo Building dependencies for %%a%%f..
        echo call %SDXROOT%\tools\razzle.cmd %%a%%f dev_build no_opt no_razacl no_ls no_oacr no_bl_ok exec build -parent -c -dir "%BUILD_DEP_DIRS%" > updatedepend%%a%%f.cmd
        cmd /c updatedepend%%a%%f.cmd 1> NUL
    )
)

:doPack

for /f "usebackq" %%i in (`dir /a:d /b %JS_ROOT%\packages`) do (
    if exist %JS_ROOT%\packages\%%i\pack.cmd (
        echo Staging and packing %%i...
        pushd %JS_ROOT%\packages\%%i
        call pack.cmd
        popd
    )
)

endlocal

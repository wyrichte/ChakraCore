@echo off
setlocal EnableDelayedExpansion

set NUGET_PACKAGE_NAME=IELibs
set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%
set NUGET_BASE_PATH=%NUGET_PACKAGE_SOURCE_DIR%\stage

rmdir /q /s %NUGET_BASE_PATH%
mkdir %NUGET_BASE_PATH%

for %%a in (x86 amd64 arm arm64) do (
    if "%%a"=="x86" (
        set objarch=i386
    ) else (
        set objarch=%%a
    )
    for %%f in (chk fre) do (
        echo Copying %NUGET_PACKAGE_NAME% built files for %%a%%f..
        robocopy %OSBuildRoot%\obj\%%a%%f\onecoreuap\inetcore\lib\scriptprojectionhost\iel3_edge\obj%%f\!objarch! %NUGET_BASE_PATH%\lib\scriptprojectionhost\iel3_edge\obj%%f\!objarch! /S /XF *.obj /NJH /NJS /NFL
        robocopy %OSBuildRoot%\obj\%%a%%f\onecoreuap\inetcore\lib\devtb\dtbhost_edge\obj%%f\!objarch!             %NUGET_BASE_PATH%\lib\devtb\dtbhost_edge\obj%%f\!objarch!             /S /XF *.obj *.pch /NJH /NJS /NFL
        robocopy %OSBuildRoot%\obj\%%a%%f\onecoreuap\inetcore\manifests\inbox\obj%%f\!objarch!                    %NUGET_BASE_PATH%\manifests\inbox\obj%%f\!objarch!                    /S                 /NJH /NJS /NFL
    )
)

call %PACKAGE_PATH%\create_package.cmd

endlocal

echo off
setlocal

set NUGET_PACKAGE_NAME=IELibs
set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%
set NUGET_BASE_PATH=%NUGET_PACKAGE_SOURCE_DIR%\stage

rmdir /q /s %NUGET_BASE_PATH%

for %%a in (x86 amd64 arm) do (
    for %%f in (chk fre) do (
        echo Copying %NUGET_PACKAGE_NAME% built files for %%a%%f..
        robocopy %SDXROOT%.obj.%%a%%f\onecoreuap\inetcore\lib\scriptprojectionhost\iel3_edge %NUGET_BASE_PATH%\lib\scriptprojectionhost\iel3_edge\ /S /XF *.obj /NJH /NJS /NFL
        robocopy %SDXROOT%.obj.%%a%%f\onecoreuap\inetcore\lib\devtb\dtbhost                  %NUGET_BASE_PATH%\lib\devtb\dtbhost\                  /S /XF *.obj *.pch /NJH /NJS /NFL
		robocopy %SDXROOT%.obj.%%a%%f\onecoreuap\inetcore\lib\devtb\dtbhost_edge             %NUGET_BASE_PATH%\lib\devtb\dtbhost_edge\             /S /XF *.obj *.pch /NJH /NJS /NFL
        robocopy %SDXROOT%.obj.%%a%%f\onecoreuap\inetcore\manifests\inbox                    %NUGET_BASE_PATH%\manifests\inbox\                    /S                 /NJH /NJS /NFL
    )
)

call %PACKAGE_PATH%\create_package.cmd

endlocal

@echo off
setlocal

set NUGET_PACKAGE_NAME=ProjectionABIs
set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\%NUGET_PACKAGE_NAME%
set NUGET_BASE_PATH=%NUGET_PACKAGE_SOURCE_DIR%\stage

rmdir /q /s %NUGET_BASE_PATH%

for %%a in (x86 amd64 arm arm64) do (
    for %%f in (chk fre) do (
        echo Copying projection binaries for %%a%%f..
        mkdir %NUGET_BASE_PATH%\%%a%%f\
        copy \\chakrafs\fs\Tools\NuGet\dependencies\%%a\TakeRegistryAdminOwnership.exe %NUGET_BASE_PATH%\%%a%%f\
        xcopy /S /Q %SDXROOT%\..\bin\%%a%%f\Projection\WinRT\*                         %NUGET_BASE_PATH%\%%a%%f\
    )
)

xcopy /Q %PUBLIC_ROOT%\sdk\winmetadata\windows.winmd        %NUGET_BASE_PATH%\ 

call %PACKAGE_PATH%\create_package.cmd

endlocal

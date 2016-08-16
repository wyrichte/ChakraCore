@echo off
setlocal

set PACKAGE_PATH=%SDXROOT%\onecoreuap\inetcore\jscript\packages
set NUGET_PACKAGE_SOURCE_DIR=%PACKAGE_PATH%\projectionabis
set NUGET_BASE_PATH=%NUGET_PACKAGE_SOURCE_DIR%\stage

for %%a in (x86 amd64 arm) do (
    for %%f in (chk fre) do (
        echo Copying projection binaries for %%a%%f..
        xcopy /Q    \\chakrafs\fs\Tools\NuGet\dependencies\%%a\TakeRegistryAdminOwnership.exe %NUGET_BASE_PATH%\%%a%%f\
        xcopy /S /Q %SDXROOT%.binaries.%%a%%f\Projection\WinRT\*                              %NUGET_BASE_PATH%\%%a%%f\
    )
)

xcopy /Q %PUBLIC_ROOT%\sdk\winmetadata\windows.winmd        %NUGET_BASE_PATH%\ 

call %PACKAGE_PATH%\create_package.cmd

endlocal

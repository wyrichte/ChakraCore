@echo off

setlocal enableextensions

set NUGET_PACKAGE_OUT=%CD%\package_output

if EXIST %NUGET_PACKAGE_OUT% (
    echo Warning: Deleting everything in %NUGET_PACKAGE_OUT%
    if "%1" NEQ "/f" (
        pause
    )
    del %NUGET_PACKAGE_OUT%\*
)

call runjs createIEDepPackage
call runjs createProjectionPackage

for /f "usebackq" %%i in (`dir /a:d /b`) do (
    if exist %%i\pack.cmd (
        echo Packaging %%i
        pushd %%i
        call pack.cmd
        popd
    )
)

endlocal

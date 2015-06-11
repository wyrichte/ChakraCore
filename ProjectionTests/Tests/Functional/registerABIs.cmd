rem -- to be run under a setlocal session --
SETLOCAL ENABLEDELAYEDEXPANSION

set _RegistrationDirectory=%_TestsDirectory%\..\Registration
set _CurrentFolder=%cd%

rem copy all latest scripts to _RegistrationDirectory
echo Copying 'reference' scripts
pushd %_RegistrationDirectory%
call getRegistrationUtilities.bat

copy /y TakeRegistryAdminOwnership.exe %_CurrentFolder%
popd

if "%1" == "-unregister" (
    set ERRORLEVEL=
    powershell.exe -ExecutionPolicy remotesigned -File %_RegistrationDirectory%\RegisterProjectionABIs.ps1 -unregister -regSvr32File %WIN_JSHOST_METADATA_BASE_PATH%\FabrikamKitchenServer.ProxyStub.dll
    if NOT ERRORLEVEL 1 (exit /b 20)
) else (
    set errorlevel=
    powershell.exe -noprofile -ExecutionPolicy remotesigned -File %_RegistrationDirectory%\RegisterProjectionABIs.ps1 -regSvr32File %WIN_JSHOST_METADATA_BASE_PATH%\FabrikamKitchenServer.ProxyStub.dll
    SET LASTERR=!errorlevel!
    ECHO powershell %_RegistrationDirectory%\RegisterProjectionABIs.ps1 RETURNED !LASTERR!
    if NOT "!LASTERR!"=="1" (exit /b 10)

    REM if exist %SYSTEMDRIVE%\windows\registrationutilities (
    REM copy %SYSTEMDRIVE%\windows\RegistrationUtilities\* %_RegistrationDirectory%
    REM )
)

exit /b 1

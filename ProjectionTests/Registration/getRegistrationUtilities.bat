@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION ENABLEEXTENSIONS 

IF EXIST "TakeRegistryAdminOwnership.exe" (
  ECHO TakeRegistryAdminOwnership.exe is already present
  goto :end
)

ECHO Check %SYSTEMDRIVE%\RegistrationUtilities folder
IF EXIST "%SYSTEMDRIVE%\RegistrationUtilities" (
    ECHO Using %SYSTEMDRIVE%\RegistrationUtilities folder
    copy /Y %SYSTEMDRIVE%\RegistrationUtilities\TakeRegistryAdminOwnership.exe .
    copy /Y %SYSTEMDRIVE%\RegistrationUtilities\* %_RegistrationDirectory%
    goto :end
)

ECHO Using the \\winbuilds\release folder
SET KEY="HKLM\SOFTWARE\Microsoft\Windows NT\CurrentVersion"

FOR /F "tokens=3" %%I IN ('REG QUERY !KEY! /v BuildLabEx') DO (
    SET VALUE=%%I
)

REM i    j k      l           m
REM 8022.0.armfre.fbl_ie_dev1.110608-1700
REM On build share, arm builds are woachk, woafre rather than armchk, armfre, so replace arm with woa
FOR /F "delims=. tokens=1-5" %%I IN ('ECHO !VALUE!') DO (
    set BuildType=%%K
    set BuildType=!BuildType:arm=woa!
    SET BUILD=\\winbuilds\release\%%L\%%I.%%J.%%M\!BuildType!
    SET BUILDBRANCH=%%L
)

SET TAKEREGISTRYEXE_RELATIVEPATH=\bin\idw\TakeRegistryAdminOwnership.exe

IF EXIST %BUILD%\bin\idw (
    copy /Y %BUILD%%TAKEREGISTRYEXE_RELATIVEPATH% .
    copy /Y %BUILD%\bin\idw\WinRTFirstPartyABIReg\* .
) ELSE (
    ECHO Directory Not Found: %BUILD%\bin\idw
    ECHO Your build is probably too old.

    SET WINBUILDS=\\winbuilds\release\!BUILDBRANCH!
    ECHO.
    ECHO Scanning !WINBUILDS! for possible match:
    ECHO.
    REM pick up the first directory with the %TAKEREGISTRYEXE_RELATIVEPATH% file
    SET FOUNDMATCH=0
    FOR /D %%D in ("!WINBUILDS!\*") DO (
        SET BUILDPATH=%%D\!BuildType!
        IF "!FOUNDMATCH!" EQU "0" (
            echo Trying to use !BUILDPATH!%TAKEREGISTRYEXE_RELATIVEPATH% if present
            IF EXIST !BUILDPATH!%TAKEREGISTRYEXE_RELATIVEPATH% (
                echo Found - using !BUILDPATH!%TAKEREGISTRYEXE_RELATIVEPATH%
                copy /Y !BUILDPATH!%TAKEREGISTRYEXE_RELATIVEPATH% .
                SET FOUNDMATCH=1
            )
        )
    )

    IF "!FOUNDMATCH!" EQU "0" (
        ECHO Unable to find TakeRegistryAdminOwnership.exe - using Winbuilds. Using known build (win8 RTM)
        SET BUILDPATH=\\winbuilds\release\WIN8_RTM\9200.16384.120725-1247\!BuildType!
        IF "!FOUNDMATCH!" EQU "0" (
            echo Trying to use !BUILDPATH!%TAKEREGISTRYEXE_RELATIVEPATH% if present
            IF EXIST !BUILDPATH!%TAKEREGISTRYEXE_RELATIVEPATH% (
                echo Found - using !BUILDPATH!%TAKEREGISTRYEXE_RELATIVEPATH%
                copy /Y !BUILDPATH!%TAKEREGISTRYEXE_RELATIVEPATH% .
                SET FOUNDMATCH=1
            )
        )
    )

    IF "!FOUNDMATCH!" EQU "0" (
        ECHO Unable to find TakeRegistryAdminOwnership.exe - error!
    )
)

:end
ENDLOCAL

@echo off

rem run_js.bat - Used by SNAP system to execute JS DRT machines 
rem                This file should be located \\iesnap\SNAP\batch
rem usage:
rem %0 ^<BucketID^> ^<Platform^> ^<BuildType^> ^<CleanIt^>
rem
rem     BucketID    ID of the JS DRT bucket to execute
rem     Platform    CPU architecture (x86, ia64, etc)
rem     BuildType   Build Type: (x86, fre, etc)
rem

SETLOCAL ENABLEEXTENSIONS
set _Error=0

if "%1" == "" goto :Usage
if "%1" == "/?" goto :Usage
if "%2" == "" goto :Usage
if "%3" == "" goto :Usage
goto :Main

:Usage
    echo %0 ^<BucketID^> ^<Platform^> ^<BuildType^>
    echo.
    echo     BucketID    ID of the JS DRT bucket to execute
    echo     Platform    CPU architecture (x86, ia64, etc)
    echo     BuildType   Build Type: (x86, fre, etc)
    echo.
    goto :End

:Main
    set BucketID=%1
    set Platform=%2
    set BuildType=%3
    set run_cmd=%systemdrive%\JSRoot\JScriptTests\tools\RunJSDRT.cmd

    echo %run_cmd% -snap -snapBucketID %BucketID% -platform %Platform% -buildType %BuildType%
    call %run_cmd% -snap -snapBucketID %BucketID% -platform %Platform% -buildType %BuildType% 2>&1
    if errorlevel 1 set _Error=1
    goto :End
    
:End
    (endlocal & exit /b %_Error%)

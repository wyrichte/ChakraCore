@echo off
SETLOCAL ENABLEEXTENSIONS
set _Error=0

if "%1" == "" goto :Usage
if "%1" == "/?" goto :Usage
if "%2" == "" goto :Usage
if "%3" == "" goto :Usage
if "%4" == "" goto :Usage
if "%5" == "" goto :Usage
goto :Main

:Usage
    echo %0 ^<Password^> ^<QueueID^> ^<JobID^> ^<Platform^> ^<BuildType^> ^<CleanIt^>
    echo.
    echo     Password    iesnapdr password
    echo     QueueID     SNAP Queue folder to search for Projection test files in
    echo     JobID       ID of the SNAP Job this drop is for
    echo     Platform    CPU architecture (x86, ia64, etc)
    echo     BuildType   Build Type: (x86, free, etc)
    echo     CleanIt     Flag: Delete files from the machine first
    echo.
    goto :End

:Main
    set Password=%1
    set QueueID=%2
    set JobID=%3
    set Platform=%4
    set BuildType=%5

    set DropShare=\\iefs\Private\Dev\IESNAP\Drops
    set CleanIt=%6

    set REPEAT=5
    echo net use "%DropShare%" /user:redmond\iesnapdr **password**
    :Loop
    net use "%DropShare%" /user:redmond\iesnapdr %Password%
    if errorlevel 1 (
        echo Net use failed with errorlevel %errorlevel%. Retrying...
        goto :LoopRetry
    )
    goto :LoopDone

    :LoopRetry
    set /A REPEAT=%REPEAT% - 1
    if %REPEAT% GTR 0 goto :Loop

    REM  failure
    exit /B 1 

    :LoopDone

    set binroot=%DropShare%\%QueueID%\%JobID%\%Platform%\%BuildType%
    set targetDir=%systemdrive%\projectiontests

    if "%CleanIt%" == "/clean" (
        rmdir /s /q %targetDir%
    )

    set setup_cmd=%binroot%\bins\Projection\snap\setup.cmd

    echo %setup_cmd% -snap %targetDir% %binroot%\bins %BuildType%
    call %setup_cmd% -snap %targetDir% %binroot%\bins %BuildType%
    if errorlevel 1 set _Error=1
    goto :End
    
:End
    (endlocal & exit /b %_Error%)

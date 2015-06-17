@echo off

setlocal ENABLEDELAYEDEXPANSION

for /F "tokens=3,4* delims= " %%A in ('REG QUERY "HKLM\Software\Microsoft\Windows NT\CurrentVersion" /v BuildLabEx') do (
    echo **********************************************************************
    echo OS BuildLabEx: %%A
    echo **********************************************************************
)

if "%_TestsDirectory%" NEQ "" (
    echo _TestsDirectory is defined - aborting!
    exit /b 10
)

set _Error=0
set _snap=0
set _drt=0
set _generate=0
set _setupOnly=0
set _snapSetup=0
set _enlistmentSetup=0
set _sdRoot=%SDXROOT%\inetcore\jscript\ProjectionTests
set _mdRoot=%sdxroot%\sdpublic\sdk\winmetadata
set _binRoot=%_NTTREE%\projection\winrt
set _testRoot=%_sdRoot%
set _snapDefaultTargetLocation=%systemdrive%
set _buildArch=
set _buildType=
set _enlistment=
set _scriptFullname=%~f0

:NextArgument
    if /i "%1" == "-generateonly" (
        set _generate=1
        goto :ArgLoop
    ) else if /i "%1" == "-snap" (
        set _testRoot=%~f2
        set _binRoot=%~f3
        set _buildType=%4
        ::Shift 3x to consume args
        for /L %%i in (1,1,3) do shift
        set _snap=1
        goto :ArgLoop
    ) else if /i "%1" == "-setuponly" (
        set _setupOnly=1
        goto :ArgLoop
    ) else if /i "%1" == "-drt" (
        set _drt=1
        goto :ArgLoop
    ) else if /i "%1" == "-usebinplacefolder" (
        set _testRoot=%_NTTREE%\Projection\localRun
        goto :ArgLoop
    ) else if /i "%1" == "-testRoot" (
        set _testRoot=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-binRoot" (
        set _binRoot=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-platform" (
        set _buildArch=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-buildType" (
        set _buildType=%2
        shift
        goto :ArgLoop
    ) else if "%1" == "" (
        goto :Main
    )

:ArgLoop
    shift
    goto :NextArgument

:Main
    if "%_generate%" == "0" (
        if "%_snap%" == "1" (
            set _snapSetup=1
        ) else if "%_drt%" == "1" (
            set _snapSetup=1
        ) else (
            set _enlistmentSetup=1
        )
    )
    
    call :OutputInfo
    
    if "%_snapSetup%" == "1" (
        call :SnapSetup
    )
    if "%_enlistmentSetup%" == "1" (
        call :EnlistmentSetup
    )
    
    goto :End
    
:OutputInfo
    echo.
    echo Setting up projection unit tests:
    echo   _testRoot=%_testRoot%
    echo   _mdRoot=%_mdRoot%
    echo   _binRoot=%_binRoot%
    echo   _sdRoot=%_sdRoot%
    echo   _snap=%_snap%
    echo   _drt=%_drt%
    echo   _buildArch=%_buildArch%
    echo   _buildType=%_buildType%
    echo   _snapSetup=%_snapSetup%
    echo   _enlistmentSetup=%_enlistmentSetup%
    echo.
    
    goto :EOF

:MakeDirIfNotExist
    if not exist %~f1 (
        echo md %~f1
        md %~f1
    )

    goto :EOF

:EnlistmentSetup
    if "%_TestsDirectory%" NEQ "" ((echo _TestsDirectory is defined - no enlistment setup possible!) & exit /b 1)
    
    if "%_sdRoot%" NEQ "%_testRoot%" (
        REM -- if %_sdRoot% defined, to a robocopy from _sdRoot to _testRoot
        robocopy %_sdRoot% %_testRoot% /mir
    )

    call :MakeDirIfNotExist %_testRoot%
    call :MakeDirIfNotExist %_binRoot%\jshost.exe.local

    xcopy /y %_mdRoot%\*.winmd %_binRoot%
    xcopy /y %_NTTREE%\jscript\pdm.dll %_binRoot%\jshost.exe.local
    xcopy /y %_NTTREE%\jscript\msdbg2.dll %_binRoot%\jshost.exe.local
    xcopy /y %_NTTREE%\jscript\pdmproxy100.dll %_binRoot%\jshost.exe.local
    xcopy %_testRoot%\Tests\projectionsGlue.js %_testRoot%\Tests\Perf
    xcopy %_testRoot%\Tests\projectionsGlue.js %_binRoot%\..
    xcopy /y %_NTTREE%\jscript\rl.exe %_binRoot%

    goto :EOF

:SnapSetup
    :: SNAP setup requires to have the JS tools folder already on the local machine - SNAP does this before calling us
    :: When SNAP makes the change to call our SetupJSDRT.cmd directly, we no longer need to do this step.
    set setupcmd=%systemdrive%\JSRoot\JScriptTests\tools\chakra.cmd test-setupsnap -trace:*.* -traceTestOutput -testOutputRelativePath:"%_buildType%_drtSetup" -baseLogDirectory:"%_snapDefaultTargetLocation%\drtJSlogs" -optin -unit -projection -buildType:%_buildType% -snapBinRoot:%_binRoot% -snapTargetRoot:%_snapDefaultTargetLocation% -sdxRoot:%_snapDefaultTargetLocation% -binRoot:%_snapDefaultTargetLocation%
    
    echo %setupcmd%
    call %setupcmd% 2>&1
    
    if errorlevel 1 goto :Error

    goto :End

:Error
    echo ERROR: Failure in %_scriptFullname%
    endlocal
    set _Error=1
    exit /b 1

:End
    endlocal & exit /b %_Error%

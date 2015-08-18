::::::::::::::::::::::::::::::::::::::::::::::::
::
:: runalltests.cmd
::
:: Runs checkin tests using jshost.exe on the path, in 2 variants:
::
:: <default>    default interpreted version
:: -native      native code generator
::
:: Logs are placed into:
::
:: Logs\interpreted
:: Logs\native
::
::::::::::::::::::::::::::::::::::::::::::::::::
@echo off

set _TestsDirectory=%cd%
setlocal ENABLEDELAYEDEXPANSION

:: If _Variants is modified make corresponding change in projection_unittest_variants array in file jscript\tools\ScriptLib\EzeAutomation.js
if "%_Variants%"=="" set _Variants=interpreted,native
set _binRoot=%_NTTREE%\projection\winrt
set WIN_JSHOST_METADATA_BASE_PATH=%_binRoot%
set _snapDefaultTargetLocation=%systemdrive%
set _RLMode=-exe
set _OS=winBlue
set _Error=0
set _OutputArgument=
set _HostFlags=
set _setup=1
set _AdditionalTags=
set _EnableRecyclerStress=1
set _snap=0
set _snapTag=""
set _drtTag=
set _drt=0
set _unregister=0
set _prerununregister=0
set _buildArch=
set _buildType=
set _scriptFullname=%~f0
set _snapBinRoot=\\iesnap\SNAP\bin

:: eg. \\iesnap\queue\FBL_IE_STAGE_DEV3\100909.iecat\Job.125515
set _snapJobDir=
 
if exist BuildType.cmd (call BuildType.cmd)
if exist BuildArch.cmd (call BuildArch.cmd)

if "%build.type%" == "" (
    set _buildType=chk
) else (
    set _buildType=%build.type%
)

if "%build.arch%" == "" (
    set _buildArch=x86
) else (
    set _buildArch=%build.arch%
)

ECHO runalltests.cmd %*

:NextArgument
if "%1" == "-?" (
    echo Usage: runalltests.cmd [-binary <path>] [-asmbase] [-asmdiff] [-logVerbose] [-setup|nosetup] [-nostress] [-snap] [-snapTests] [-win8 or -winBlue] [-force] [-snapJobDir <dir>]
    echo.
    echo Use the -snap parameter to minic SNAP behavior with unregistration on exit
    echo  or the -snapTests to run the SNAP test w/o unregistration on exit
    echo.
    echo Use -win8 or -winBlue to set the windows OS version. By default, -winBlue is used.
    echo.
    echo Use -force to force ABI re-registration
    echo.

    exit /b 0
) else if /i "%1" == "-force" (
    echo -FORCE: force unregister/re-register
    set _prerununregister=1
    shift
    goto ArgLoop
) else if /i "%1" == "-binary" (
    set _JCBinaryArgument=-binary %2
    shift
    goto ArgLoop
) else if /i "%1" == "-asmbase" (
    set _RLMode=-asmbase
    set _Variants=native
    goto ArgLoop
) else if /i "%1" == "-asmdiff" (
    set _RLMode=-asmdiff
    set _Variants=native
    goto ArgLoop
) else if /i "%1" == "-variants" (
    set _Variants=%~2
    shift
    goto ArgLoop
) else if /i "%1" == "-logverbose" (
    set _OutputArgument=%_OutputArgument% -logverbose
    goto ArgLoop
) else if /i "%1" == "-setup" (
    set _setup=1
    goto ArgLoop
) else if /i "%1" == "-nosetup" (
    set _setup=0
    goto ArgLoop
) else if /i "%1" == "-nostress" (
    set _EnableRecyclerStress=0
    set _AdditionalTags=%_AdditionalTags% -nottags exclude_nostress
    goto ArgLoop
) else if /i "%1" == "-snapTests" (
    set _snap=0
    set _setup=0
    set _snapTag=-nottags exclude_snap
    goto ArgLoop
) else if /i "%1" == "-snap" (
    set _snap=1
    set _setup=0
    set _OutputArgument=%_OutputArgument% -logverbose
    set _snapTag=-nottags exclude_snap
    set _unregister=1
    goto ArgLoop
) else if /i "%1" == "-drt" (
    set _drt=1
    set _setup=0
    set _drtTag=-nottags exclude_drt
    set _unregister=1
    goto ArgLoop
) else if /i "%1" == "-winBlue" (
    set _OS=winBlue
    goto ArgLoop
) else if /i "%1" == "-win8" (
    set _OS=win8
    goto ArgLoop
) else if /i "%1" == "-platform" (
    set _buildArch=%2
    shift
    goto :ArgLoop
) else if /i "%1" == "-buildType" (
    set _buildType=%2
    shift
    goto :ArgLoop
) else if /i "%1" == "-testsDirectory" (
    set _TestsDirectory=%2
    shift
    goto :ArgLoop
) else if /i "%1" == "-binaryRoot" (
    set _binRoot=%2
    shift
    goto :ArgLoop
) else if /i "%1" == "-snapJobDir" (
    set _snapJobDir=%2
    shift
    goto :ArgLoop
) else if "%1" == "" (
    goto StartScript
)

:ArgLoop
shift
goto :NextArgument

:StartScript
set WIN_JSHOST_METADATA_BASE_PATH=%_binRoot%
set _JSHostEXE=%_nttree%\jscript\jshost.exe
set _JCBinaryArgument=-binary %_JSHostEXE%

call :OutputInfo

if "%_snap%" == "1" (
    call :SnapRun

    if errorlevel 1 goto :Error

    goto :End
)

del /s /q Logs\rl*.log >nul 2>&1

if not exist "%_JSHostEXE%" (
    echo File %_JSHostEXE% doesn't exist - exit
    exit /b 10
)

if "%_prerununregister%" == "1" (
    pushd %_TestsDirectory%\Functional
    call registerABIs.cmd -unregister
    popd
)

if %_setup% == 1 (
    if "%object_root%" NEQ "" (
        ::jsenv modified to copy built rl
        copy %object_root%\inetcore\jscript\unittest\ut_rl\%_BuildAlt%\rl.exe %_nttree%\jscript
        set "path=%path%;%_nttree%\jscript;"
    )

    call setup.cmd
    pushd %_TestsDirectory%\Functional
    call registerABIs.cmd
    popd %_TestsDirectory%
)else (
    set "path=%path%;%_TestsDirectory%;"

    :: those will fail on Win7 - but this is no longer supported
    pushd %_TestsDirectory%\Functional
    set errorlevel=
    call registerABIs.cmd
    SET LASTERR=!errorlevel!
    popd
    ECHO registerABIs.cmd RETURNED !LASTERR!
    if NOT "!LASTERR!"=="1" (exit /b 1)
)

::Add binary path (needed to locate jdtest.exe, etc.)
set "path=%path%;%_binRoot%"

for %%i in (%_Variants%) do (
    set _TESTCONFIG=%%i
    call :RunOneConfig
)

echo.
echo.

if "%_unregister%" == "1" (
    pushd %_TestsDirectory%\Functional
    call registerABIs.cmd -unregister
    popd
)

for %%i in (%_Variants%) do (
    echo ######## Logs for %%i variant ########
    type Logs\%%i\rl.log
    echo.
)

exit /b %_Error%

:SnapRun
    echo Executing SNAP jscript DRT bucket including:
    echo   Run JSProjectionTests
    echo   Run jscript dev unit tests bytecodeserialized variant
    echo.

    :: SNAP run requires to have the JS tools folder already on the local machine - SNAP does this before calling us
    :: When SNAP makes the change to call our RunJSDRT.cmd directly, we no longer need to do this step.
	
    set runcmd=%systemdrive%\JSRoot\JScriptTests\tools\chakra.cmd test -trace:*.* -traceTestOutput -doSnapSetup- -optin -unit:"-html -variants:interpreted;dynapogo" -projection -buildType:%_buildType% -platform:%_buildArch% -snapBinRoot:%_binRoot% -snapTargetRoot:%_snapDefaultTargetLocation% -sdxRoot:%_snapDefaultTargetLocation% -binRoot:%_snapDefaultTargetLocation% -snap -drt -baseLogDirectory:"%_snapDefaultTargetLocation%\logs"

    echo %runcmd%
    call %runcmd% 2>&1

    if errorlevel 1 (
        :: In-case of an error, check dumps under the temp folder, convert them to cab files and then copy them to shared folder.
        if exist %_snapJobDir% (
            echo Creating cab files
            if not exist "%_snapBinRoot%\cabarc.exe" (
                echo %_snapBinRoot%\cabarc.exe doesn't exist.
            )
            for /F %%a in ('dir /B %tmp%\*.dmp') do (
                echo %_snapBinRoot%\cabarc.exe N %tmp%\%%a.cab %tmp%\%%a
                %_snapBinRoot%\cabarc.exe N %tmp%\%%a.cab %tmp%\%%a
                echo copying %tmp%\%%a.cab to %_snapJobDir%
                copy /y %tmp%\%%a.cab %_snapJobDir%
                echo deleting %tmp%\%%a and %tmp%\%%a.cab
                del /f %tmp%\%%a
                del /f %tmp%\%%a.cab
            )
        )
        goto :Error
    )

    goto :End

:OutputInfo
    echo Executing projection unit tests:
    echo   _snap=%_snap%
    echo   _buildArch=%_buildArch%
    echo   _buildType=%_buildType%
    echo   _binRoot=%_binRoot%
    echo   WIN_JSHOST_METADATA_BASE_PATH=%WIN_JSHOST_METADATA_BASE_PATH%
    echo   _JSHostEXE=%_JSHostEXE%
    echo   _TestsDirectory=%_TestsDirectory%
    echo   _JCBinaryArgument=%_JCBinaryArgument%
    echo   _variants=%_variants%
    echo   _OS=%_OS%
    echo.

    goto :EOF

:::::::::::::::::::::::::::::::::::::::::::::::::
:RunOneConfig
::
:: IMPORTANT: this subroutine returns pass/fail in the _Error environment
:: variable.  Don't surround this call with setlocal/endlocal, or changes
:: to that variable will not be persisted, and failures will not be reflected
:: to SNAP.
::

echo ############# Starting %_TESTCONFIG% variant #############
del /q Logs\rl*.log >nul 2>&1

md Logs\%_TESTCONFIG% >nul 2>&1

set _OLD_CC_FLAGS=%EXTRA_CC_FLAGS%
if "%_TESTCONFIG%"=="interpreted" set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -NoNative
if "%_TESTCONFIG%"=="native"      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceNative -off:simpleJit -bgJitDelay:0

call runtests.cmd %_OutputArgument% %_JCBinaryArgument% %_snapTag% %_drtTag% -nottags fails_%_TESTCONFIG% -nottags exclude_%_TESTCONFIG% -nottags exclude_%_buildType% -nottags exclude_%_buildArch% -nottags exclude_%_OS% -nottags fail_%_OS% %_AdditionalTags% %_RLMode%
if errorlevel 1 set _Error=1
if not errorlevel 0 set _Error=1

move Logs\*.log Logs\%_TESTCONFIG% >nul 2>&1
set EXTRA_CC_FLAGS=%_OLD_CC_FLAGS%

goto :EOF

:Error
    echo ERROR: Failure in %_scriptFullname%
    endlocal
    set _Error=1
    exit /b 1

:End
    exit /b %_Error%

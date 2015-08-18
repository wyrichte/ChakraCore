@echo off

set _Error=0

setlocal ENABLEDELAYEDEXPANSION

set _bucketIndex=0
set _buildType=
set _buildArch=
set _targetRoot=%systemdrive%\jscript
set _jscriptRoot=%_targetRoot%\jscript
set _projectionTestsRoot=
set _toolsRoot=
set _unittestRoot=
set _coreUnittestRoot=
set _runProjectionTests=
set _runJSRTUnitTests=
set _runUnitTests=
set _snap=
set _drt=
set _nightly=
set _variants=
set _os=
set _scriptFullname=%~f0


@rem By default don't run html tests. runHtmlUnitTests resets this.
set _htmltags=-nottags html

:NextArgument
    if "%1" == "-?" (
        echo Usage: RunJSDRT.cmd [-snapBucketID ^<id^>] [-jscriptRoot ^<path^>] [-targetRoot ^<path^>] [-platform ^<arch^>] [-buildType ^<type^>]
        echo.
        echo -snapBucketID value needs to be a JS DRT bucket index as defined in
        echo the IESNAP model file ^(\\iesnap\SNAP\model\fbl_ie_script_dev.model.xml^).
        echo.
        echo -jscriptRoot is a path to the location on the local machine in which the
        echo jscript binaries are located.
        echo ^(default: %_jscriptRoot%^)
        echo.
        echo -targetRoot is a path to the location on the local machine in which the
        echo test collateral is located.
        echo ^(default: %_targetRoot%^)
        echo.
        exit /b 0
    ) else if /i "%1" == "-snapBucketID" (
        :: JS DRT buckets begin at 50 in SNAP. Adjust our index to a simple range [1-n].
        :: See the IESNAP model file - \\iesnap\SNAP\model\fbl_ie_script_dev.model.xml - for
        :: more information on the JS DRT bucket indices.
        set /a _bucketIndex=%2 - 49
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
    ) else if /i "%1" == "-targetRoot" (
        set _targetRoot=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-jscriptRoot" (
        set _jscriptRoot=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-variants" (
        set _variants=-variants %2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-snap" (
        set _snap=-snap
        goto :ArgLoop
    ) else if /i "%1" == "-drt" (
        set _drt=-drt
        goto :ArgLoop
    ) else if /i "%1" == "-nightly" (
        set _nightly=-nightly
        goto :ArgLoop
    ) else if /i "%1" == "-runProjectionTests" (
        set _runProjectionTests=1
        goto :ArgLoop
    ) else if /i "%1" == "-runJSRTUnitTests" (
        set _runJSRTUnitTests=1
        goto :ArgLoop
    ) else if /i "%1" == "-runUnitTests" (
        set _runUnitTests=1
        goto :ArgLoop
    ) else if /i "%1" == "-runHtmlUnitTests" (
        REM Html tests that are run with jshost are failing in SNAP job - this bug needs to be fixed
        REM set _runUnitTests=1
        REM set _htmltags=-tags html
        goto :ArgLoop
    ) else if /i "%1" == "-win7" (
        set _os=-win7
        goto :ArgLoop
    ) else if /i "%1" == "-win8" (
        set _os=-win8
        goto :ArgLoop
    ) else if /i "%1" == "-win10" (
        set _os=-win10
        goto :ArgLoop
    ) else if /i "%1" == "-winBlue" (
        set _os=-winBlue
        goto :ArgLoop
    ) else if "%1" == "" (
        goto :DetectOS
    )

:ArgLoop
    shift
    goto :NextArgument

:DetectOS
    if not "%_os%" == "" (
        echo Skipping OS detection and using %_os%
        goto :Main
    )

    ver | find "10.0"
    if %errorlevel% == 0 (
        set _os=-win10
        echo Detected Windows 10
        goto :Main
    )
    ver | find "6.3"
    if %errorlevel% == 0 (
        set _os=-winBlue
        echo Detected Windows Blue
        goto :Main
    )
    ver | find "6.2"
    if %errorlevel% == 0 (
        set _os=-win8
        echo Detected Windows 8
        goto :Main
    )
    ver | find "6.1"
    if %errorlevel% == 0 (
        set _os=-win7
        echo Detected Windows 7
        goto :Main
    )

    echo Unknown host OS.
    goto :Error

:Main
    set _toolsRoot=%_targetRoot%\inetcore\jscript\tools
    set _unittestRoot=%_targetRoot%\inetcore\jscript\unittest
    set _coreUnittestRoot=%_targetRoot%\inetcore\jscript\core\test
    set _projectionTestsRoot=%_targetRoot%\ProjectionTests

    :: Use the bucket index to figure out which suite of tests to run.
    if %_bucketIndex% == 0 (
        :: No-op, we didn't specify a bucket
        set _bucketIndex=0
    ) else if %_bucketIndex% == 7 (
        set _runJSRTUnitTests=1
    ) else if %_bucketIndex% == 9 (
        set _runProjectionTests=1
    ) else if %_bucketIndex% lss 6 (
        set _runUnitTests=1
    )

    call :OutputInfo

    if "%_runProjectionTests%" == "1" (
        call :RunProjectionTests
        if errorlevel 1 goto :Error
    )
    if "%_runUnitTests%" == "1" (
        call :RunUnitTests
        if errorlevel 1 goto :Error
    )
    if "%_runJSRTUnitTests%" == "1" (
        call :RunJSRTUnitTests
        if errorlevel 1 goto :Error
    )

    echo.
    echo Successfully executed DRT!
    echo.

    goto :End

:OutputInfo
    echo Executing JS DRT Bucket:
    echo   _snap=%_snap%
    echo   _drt=%_drt%
    echo   _nightly=%_nightly%
    echo   _buildArch=%_buildArch%
    echo   _buildType=%_buildType%
    echo   _bucketIndex=%_bucketIndex%
    echo   _jscriptRoot=%_jscriptRoot%
    echo   _toolsRoot=%_toolsRoot%
    echo   _unittestRoot=%_unittestRoot%
    echo   _coreUnittestRoot=%_coreUnittestRoot%
    echo   _projectionTestsRoot=%_projectionTestsRoot%
    echo   _runProjectionTests=%_runProjectionTests%
    echo   _runJSRTUnitTests=%_runJSRTUnitTests%
    echo   _runUnitTests=%_runUnitTests%
    echo   _variants=%_variants%
    echo   _os=%_os%
    echo.

    goto :End

:RunProjectionTests
    set _jshostRoot=%_projectionTestsRoot%\JSHost
    set _testCmd=%_projectionTestsRoot%\Tests\runalltests.cmd -snapTests %_drt% -platform %_buildArch% -buildType %_buildType% %_variants% %_os% -logverbose -binaryRoot %_jshostRoot%

    pushd %_projectionTestsRoot%\Tests
    echo %_testCmd%
    call %_testCmd% 2>&1
    popd

    if errorlevel 1 goto :Error

    goto :End

:RunUnitTests
    set _tags=
    if %_bucketIndex% neq 0 (
        set _tags=-dirtags include_drt%_bucketIndex%
    )

    set _testCmd=%_unittestRoot%\RunAllRLTests.cmd %_snap% %_drt% %_nightly% -platform %_buildArch% -buildType %_buildType% -toolsRoot %_toolsRoot% -binaryRoot %_jscriptRoot% %_variants% %_tags% %_os% %_htmltags%

    pushd %_coreUnittestRoot%
    echo %_testCmd%
    call %_testCmd% 2>&1
    popd

    if errorlevel 1 goto :Error

    pushd %_unittestRoot%
    echo %_testCmd%
    call %_testCmd% 2>&1
    popd

    if errorlevel 1 goto :Error

    goto :End

:RunJSRTUnitTests
    set _testCmd=%_toolsRoot%\RunAllJSRTTests.cmd %_snap% %_drt% -toolsRoot %_toolsRoot% -binaryRoot %_jscriptRoot%

    pushd %_toolsRoot%
    echo %_testCmd%
    call %_testCmd% 2>&1
    popd

    if errorlevel 1 goto :Error

    goto :End

:Error
    echo ERROR: Failure in %_scriptFullname%
    endlocal
    set _Error=1
    exit /b 1

:End
    endlocal & exit /b %_Error%

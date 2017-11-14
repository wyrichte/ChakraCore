@echo off

setlocal ENABLEDELAYEDEXPANSION

set _Error=0
set _snapBinRoot=\\iesnap\SNAP\bin
set _bucketIndex=0
set _targetDir=
set _binRoot=
set _additionalBinRoot=
set _buildType=
set _buildArch=
set _targetRoot=%systemdrive%\jscript
set _jscriptRoot=%_targetRoot%\jscript
set _testCab=
set _projectionTestCab=
set _setupProjectionTests=
set _setupJSRTUnitTests=
set _setupUnitTests=
set _snap=
set _drt=
set _nightly=
set _scriptFullname=%~f0

:NextArgument
    if "%1" == "-?" (
        echo Usage: SetupJSDRT.cmd [-snapBucketID ^<id^>] [-binaries ^<path^>] [-jscriptRoot ^<path^>] [-targetRoot ^<path^>] [-platform ^<arch^>] [-buildType ^<type^>]
        echo.
        echo -snapBucketID value needs to be a JS DRT bucket index as defined in 
        echo the IESNAP model file ^(\\iesnap\SNAP\model\fbl_ie_script_dev.model.xml^).
        echo.
        echo -binaries is a full path to the binaries to setup on the DRT machine. 
        echo The binaries may be built as part of a SNAP job, enlistment, official 
        echo build or otherwise.
        echo SNAP build requires drop location ^(complete with build type \ architecture^).
        echo ex: \\iefs\Private\Dev\IESNAP\Drops\FBL_IE_SCRIPT_DEV\^<jobid^>\x86\fre\bins
        echo.
        echo -jscriptRoot is a path to the location on the local machine into which we 
        echo will copy the jscript binaries.
        echo ^(default: %_jscriptRoot%^)
        echo.
        echo -targetRoot is a path to the location on the local machine into which we 
        echo will copy the test collateral.
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
    ) else if /i "%1" == "-binaries" (
        set _binRoot=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-additionalBinaries" (
        set _additionalBinRoot=%2
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
    ) else if /i "%1" == "-jscriptRoot" (
        set _jscriptRoot=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-targetRoot" (
        set _targetRoot=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-snapBinRoot" (
        set _snapBinRoot=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-testCab" (
        set _testCab=%2
        shift
        goto :ArgLoop
    ) else if /i "%1" == "-projectionTestCab" (
        set _projectionTestCab=%2
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
    ) else if /i "%1" == "-setupProjectionTests" (
        set _setupProjectionTests=1
        goto :ArgLoop
    ) else if /i "%1" == "-setupJSRTUnitTests" (
        set _setupJSRTUnitTests=1
        goto :ArgLoop
    ) else if /i "%1" == "-setupUnitTests" (
        set _setupUnitTests=1
        goto :ArgLoop
    ) else if "%1" == "" (
        goto :Main
    )

:ArgLoop
    shift
    goto :NextArgument

:Main
    set _projectionTestsRoot=%_targetRoot%\ProjectionTests
    
    :: Use the bucket index to figure out which suite of tests to setup.    
    if %_bucketIndex% == 0 (
        :: No-op, we didn't specify a bucket
        set _bucketIndex=0
    ) else if %_bucketIndex% == 7 (
        set _setupJSRTUnitTests=1
    ) else if %_bucketIndex% == 9 (
        set _setupProjectionTests=1
    ) else if %_bucketIndex% lss 6 (
        set _setupUnitTests=1
    )
    
    if "%_testCab%" == "" (
        :: We didn't specify path to JScriptTestCollateral.cab - find it in the binaries root
        set _testCab=%_binRoot%\JScriptTestCollateral.cab
    )
    if "%_projectionTestCab%" == "" (
        :: We didn't specify path to projection Tests.cab - find it in the binaries root
        set _projectionTestCab=%_binRoot%\Projection\Tests.cab
    )

    call :OutputInfo
    call :SetupCommon

    if "%_setupProjectionTests%" == "1" (
        call :SetupProjection
        if errorlevel 1 goto :Error
    )
    if "%_setupUnitTests%" == "1" (
        call :SetupUnitTests
        if errorlevel 1 goto :Error
    )
    if "%_setupJSRTUnitTests%" == "1" (
        call :SetupJSRT
        if errorlevel 1 goto :Error
    )

    echo.
    echo Successfully setup DRT collateral!
    echo.
    
    goto :End
    
:OutputInfo
    echo Setting up JS DRT Bucket:
    echo   _snap=%_snap%
    echo   _drt=%_drt%
    echo   _nightly=%_nightly%
    echo   _binRoot=%_binRoot%
    echo   _additionalBinRoot=%_additionalBinRoot%
    echo   _targetRoot=%_targetRoot%
    echo   _jscriptRoot=%_jscriptRoot%
    echo   _projectionTestsRoot=%_projectionTestsRoot%
    echo   _buildArch=%_buildArch%
    echo   _buildType=%_buildType%
    echo   _bucketIndex=%_bucketIndex%
    echo   _testCab=%_testCab%
    echo   _projectionTestCab=%_projectionTestCab%
    echo   _setupProjectionTests=%_setupProjectionTests%
    echo   _setupJSRTUnitTests=%_setupJSRTUnitTests%
    echo   _setupUnitTests=%_setupUnitTests%
    echo.
    
    goto :EOF
    
:CleanDir
    :: Remove any existing files in %1
    echo rmdir /s /q %~f1
    rmdir /s /q %~f1
    
    call :MakeDirIfNotExist %~f1
    
    goto :EOF

:MakeDirIfNotExist
    if not exist %~f1 (
        echo md %~f1
        md %~f1
    )
    goto :EOF
    
:MakeConfigs
    setlocal
    
    for /f "tokens=1*" %%i in ("%*") do (
        set _target=%%~i
        set _args=%%~j
    )
    echo Making %_target% configs
    
    if exist %_projectionTestsRoot%\%_target%\jshost.exe.config (
        echo del /f %_projectionTestsRoot%\%_target%\jshost.exe.config
        del /f %_projectionTestsRoot%\%_target%\jshost.exe.config
    )
    
    echo copy /y %_binRoot%\Projection\DevTests\Configs\jshost.exe.config %_projectionTestsRoot%\%_target%
    copy /y %_binRoot%\Projection\DevTests\Configs\jshost.exe.config %_projectionTestsRoot%\%_target%
    
    if exist %_projectionTestsRoot%\%_target%\jscript.config (
        echo del /f %_projectionTestsRoot%\%_target%\jscript.config
        del /f %_projectionTestsRoot%\%_target%\jscript.config
    )
    
    echo echo %_args% ^> %_projectionTestsRoot%\%_target%\jscript.config
    echo %_args% > %_projectionTestsRoot%\%_target%\jscript.config
    
    echo.
    
    endlocal
    
    goto :EOF

:CheckRoboCopyErrorLevel
    if errorlevel 8 (
        echo :: ERROR: Robocopy failed to copy files. Exiting.
        goto :Error
    )
    if errorlevel 4 (
        echo :: WARNING: Robocopy reported mismatched files. Consider cleaning the destination directory.
        goto :Error
    )
    :: Robocopy returns between 0, 1, or 2 on successful copy - convert those to 0
    exit /b 0

:CheckXCopyErrorLevel
    if errorlevel 4 (
        echo :: ERROR: XCopy failed; aborting.
        goto :Error
    ) else if errorlevel 2 (
        echo :: ABORT: User cancelled, leaving script.
        goto :Error
    ) else if errorlevel 1 (
        echo :: ERROR: Could not find the source file. The script may need updating.
        goto :Error
    )
    goto :EOF

:SetupCommon
    call :CleanDir %_jscriptRoot%
    call :MakeDirIfNotExist %_jscriptRoot%

    echo Copying common binaries
    robocopy %_binRoot% %_jscriptRoot% *.dll *.exe *.mui *.tlb *.pdb /njh /njs /ndl /purge /xx /np

    call :CheckRoboCopyErrorLevel
    echo.
    
    if errorlevel 1 goto :Error
    
    if not "%_additionalBinRoot%" == "" (
        echo Copying additional binaries
        robocopy %_additionalBinRoot% %_jscriptRoot% *.dll *.exe *.mui *.tlb *.pdb /njh /njs /ndl /purge /xx /np

        call :CheckRoboCopyErrorLevel
        echo.

        if errorlevel 1 goto :Error
    )
    
    goto :End

:SetupJSRT
    set _sourceDir=%_binRoot%\jsrt\unittest
    set _targetDir=%_jscriptRoot%\jsrt\unittest
    
    call :CleanDir %_targetDir%
    
    echo Copying JSRT binaries
    robocopy %_sourceDir% %_targetDir% *.dll *.exe *.mui *.tlb *.js /njh /njs /ndl /purge /xx /np

    call :CheckRoboCopyErrorLevel
    echo.
    
    if errorlevel 1 goto :Error
    
    echo Successfully setup JSRT unit tests
    echo.
    
    goto :EOF
    
:SetupProjection
    set _targetDir=%_projectionTestsRoot%
    set _mdRoot=%_targetDir%\winmetadata
    
    call :CleanDir %_targetDir%

    call :MakeDirIfNotExist %_targetDir%
    call :MakeDirIfNotExist %_targetDir%\JsHost
    call :MakeDirIfNotExist %_targetDir%\JsHost\JsHost.exe.local

    echo Extracting test collateral
    if exist %_projectionTestCab% (

        echo Needs 7-zip installed on the test machine and on the %path%.
        echo 7z x -r -aoa -y %_projectionTestCab% -o%_targetDir%\ *.*
        7z x -r -aoa -y %_projectionTestCab% -o%_targetDir%\ *.*

        if not exist %_targetDir%\Tests\Functional\AsyncDebug.js (
            echo Expected files are not found after extracting Tests.cab. Aborting.
            goto :Error
        )
    )
    echo.

    echo Copying ABIs
    xcopy /y %_binRoot%\Projection\winrt\*.* %_targetDir%\JsHost
    call :CheckXCopyErrorLevel    
    xcopy /y %_mdRoot%\*.winmd %_targetDir%\JsHost
    call :CheckXCopyErrorLevel
    echo.

    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\Configurable
    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\Functional
    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\HeapDump
    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\Perf
    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\MemoryTracing
    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\MetadataParsing
    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\RecyclerStress
    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\ScriptErrorTests
    xcopy /y %_targetDir%\JsHost\*.* %_targetDir%\Tests\Versioning
    echo.

    echo Copying test binaries
    xcopy /y %_binRoot%\chakratest.dll %_targetDir%\JsHost
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\chakradiagtest.dll %_targetDir%\JsHost
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\jshost.exe %_targetDir%\JsHost
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\jdtest.exe %_targetDir%\JsHost
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\JsEtwConsole.exe %_targetDir%\JsHost
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\pdm.dll %_targetDir%\JsHost\JsHost.exe.local
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\msdbg2.dll %_targetDir%\JsHost\JsHost.exe.local
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\pdmproxy100.dll %_targetDir%\JsHost\JsHost.exe.local
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\rl.exe %_targetDir%\Tests
    call :CheckXCopyErrorLevel
    echo.

    echo Write build type
    echo echo set build.type=%_buildType% ^> %_targetDir%\Tests\BuildType.cmd
    echo set build.type=%_buildType%> %_targetDir%\Tests\BuildType.cmd
    echo.

    set _defaultConfig=-HostType:2
    call :MakeConfigs JsHost "%_defaultConfig%"
    
    if errorlevel 1 goto :Error
    
    echo Successfully setup projection unit tests
    echo.

    goto :EOF
    
:SetupUnitTests
    set _targetDir=%_targetRoot%\onecoreuap\inetcore\jscript
    
    call :CleanDir %_targetDir%
    call :MakeDirIfNotExist %_jscriptRoot%\jshost.exe.local

    echo Copying binaries
    xcopy /y %_binRoot%\pdm.dll %_jscriptRoot%\jshost.exe.local
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\msdbg2.dll %_jscriptRoot%\jshost.exe.local
    call :CheckXCopyErrorLevel
    xcopy /y %_binRoot%\pdmproxy100.dll %_jscriptRoot%\jshost.exe.local
    call :CheckXCopyErrorLevel
    echo.

    echo Extracting test collateral
    if exist %_testCab% (
        echo Needs 7-zip installed on the test machine and on the %path%.
        echo 7z x -r -aoa -y %_testCab% -o%_targetDir%\ *.*
        7z x -r -aoa -y %_testCab% -o%_targetDir%\ *.*

        if not exist %_targetDir%\Tools\runjs.bat (
            echo Expected files are not found after extracting JScriptTestCollateral.cab. Aborting.
            goto :Error
        )
    )
    echo.

    echo Setting up test hosts
    call %_targetDir%\Tools\runjs.bat setupTesthost %_binRoot% %_jscriptRoot%
    if errorlevel 1 goto :Error
    
    call %_targetDir%\Tools\runjs.bat setupJdTest %_binRoot% %_jscriptRoot%
    if errorlevel 1 goto :Error
    
    call %_targetDir%\Tools\runjs.bat setupJsHostTest %_binRoot% %_jscriptRoot%
    if errorlevel 1 goto :Error
    echo.
    
    echo Successfully setup dev unit tests
    echo.

    goto :EOF

:Error
    echo ERROR: Failure in %_scriptFullname%
    endlocal
    set _Error=1
    exit /b 1

:End
    endlocal & exit /b %_Error%

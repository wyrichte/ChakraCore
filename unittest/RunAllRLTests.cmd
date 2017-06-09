::::::::::::::::::::::::::::::::::::::::::::::::
::
:: RunAllRLTests.cmd
::
:: Runs checkin tests using the JC.exe on the path, in 2 variants:
::
:: -maxInterpretCount:1 -maxSimpleJitRunCount:1 -bgjit-
:: <dynapogo>
::
:: Logs are placed into:
::
:: logs\interpreted
:: logs\dynapogo
::
:: User specified variants:
:: logs\forcedeferparse
:: logs\nodeferparse
:: logs\forceundodefer
:: logs\bytecodeserialized (serialized to byte codes)
:: logs\forceserialized (force bytecode serialization internally)
::
:: As a precheckin test, run jscript\tools\RunAllTests.cmd
:: Don't run this directly.
::
::::::::::::::::::::::::::::::::::::::::::::::::
@echo off
setlocal

if not exist %cd%\rlexedirs.xml (
    echo RunAllRLTests.cmd Error: rlexedirs.xml not found in current directory.
    echo RunAllRLTests.cmd must be run from a unit test root directory.
    exit /b 0
)

set _runAllRLTestsDir=%~dp0
set _buildType=%build.type%
set _buildArch=%build.arch%
set _Variants=
set _TAGS=
set _NOTTAGS=
set _DIRNOTTAGS=
set _DIRTAGS=
set _binaryRoot=%_nttree%\jscript
set _toolsRoot=%sdxroot%\onecoreuap\inetcore\jscript\tools
set _full=
set _drt=
set _snap=
set _nightly=
set _rebase=
set _ExtraVariants=
set _logsRoot=%cd%\logs
set _ccChangeList=
set _GumshoePath=%ProgramFiles%\Gumshoe
set _GumshoeExe=%ProgramFiles%\Gumshoe\gumshoe.exe
set _MagellanPath=%SystemDrive%\ChakraMagellan
set _DpkFile=ChakraCC_
set _GumshoeSetupPath=\\ocentral\products\Gumshoe\latest\setup\x64\gumshoe.msi
set _MagellanSetupPath=\\codecovfs01\Magellan_pre_Release\Latest\amd64fre
set _runT262=
set _runDefault=1
set _t262OnlyMode=0

:NextArgument
if "%1" == "-?" (
    echo Usage: runalltests.cmd [-binary <path>]
    exit /b 0
) else if /i "%1" == "-variants" (
    set _Variants=%~2
    shift
    goto ArgLoop
) else if /i "%1" == "-extraVariants" (
:: Extra variants are specified by the user but not run
:: by default.  Shorthand versions for specific variants
:: can be found at the end of the loop.
    if "%_ExtraVariants%" == "" (
        set _ExtraVariants=%~2
    ) else (
        set _ExtraVariants=%_ExtraVariants%,%~2
    )
    shift
    goto ArgLoop
) else if /i "%1" == "-binary" (
    set _JCBinaryArgument=-binary %2
    shift
    goto ArgLoop
) else if /i "%1" == "-asmbase" (
    set _RLMode=-asmbase
    set _Variants=default
    goto ArgLoop
) else if /i "%1" == "-asmdiff" (
    set _RLMode=-asmdiff
    set _Variants=default
    goto ArgLoop
) else if /i "%1" == "-dirs" (
    set _DIRS=-dirs %2
    shift
    goto ArgLoop
) else if /i "%1" == "-win7" (
    set TARGET_OS=win7
    goto ArgLoop
) else if /i "%1" == "-win8" (
    set TARGET_OS=win8
    goto ArgLoop
) else if /i "%1" == "-winBlue" (
    set TARGET_OS=winBlue
    goto ArgLoop
) else if /i "%1" == "-win10" (
    set TARGET_OS=win10
    goto ArgLoop
) else if /i "%1" == "-nottags" (
    set _NOTTAGS=%_NOTTAGS% -nottags %2
    shift
    goto ArgLoop
) else if /i "%1" == "-tags" (
    set _TAGS=%_TAGS% -tags %2
    shift
    goto ArgLoop
) else if /i "%1" == "-dirtags" (
    set _DIRTAGS=%_DIRTAGS% -dirtags %2
    shift
    goto ArgLoop
) else if /i "%1" == "-dirnottags" (
    set _DIRNOTTAGS=%_DIRNOTTAGS% -dirnottags %2
    shift
    goto ArgLoop
) else if /i "%1" == "-snap" (
    set _snap=1
    set _NOTTAGS=%_NOTTAGS% -nottags exclude_snap
    goto ArgLoop
)else if /i "%1" == "-full" (
    set _full=1
    goto ArgLoop
)else if /i "%1" == "-drt" (
    set _drt=1
    set _NOTTAGS=%_NOTTAGS% -nottags exclude_drt
    goto ArgLoop
) else if /i "%1" == "-nightly" (
    set _nightly=1
    set _NOTTAGS=%_NOTTAGS% -nottags exclude_nightly
    goto ArgLoop
) else if /i "%1" == "-rebase" (
    set _rebase=-rebase
    goto ArgLoop
) else if /i "%1" == "-rundebug" (
    set _RUNDEBUG=1
    goto ArgLoop
) else if /i "%1" == "-platform" (
    set _buildArch=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-buildType" (
    set _buildType=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-binaryRoot" (
    set _binaryRoot=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-toolsRoot" (
    set _toolsRoot=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-codeCoverage" (
    set _ccChangeList=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-cc" (
    set _ccChangeList=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-t262" (
    set _runT262=1
    goto ArgLoop

:: Defined here are shorthand versions for specifying
:: extra variants when running.
) else if /i "%1" == "-parser" (
    if "%_ExtraVariants%" == "" (
        set _ExtraVariants=forcedeferparse,nodeferparse,forceundodefer
    ) else (
        set _ExtraVariants=%_ExtraVariants%,forcedeferparse,nodeferparse,forceundodefer
    )
    goto ArgLoop
) else if /i "%1" == "-serialization" (
    if "%_ExtraVariants%" == "" (
        set _ExtraVariants=bytecodeserialized,forceserialized
    ) else (
        set _ExtraVariants=%_ExtraVariants%,bytecodeserialized,forceserialized
    )
    goto ArgLoop
) else if "%1" == "" (
    goto StartScript
) else (
    echo Unknown argument: %1
    goto :eof
)

:ArgLoop
if not "%1" == "-t262" (
    set _runDefault=0
)
shift
goto :NextArgument

:StartScript

if NOT "%_full%" == "1" (
    set _NOTTAGS=%_NOTTAGS% -nottags Slow
)

if "%_runDefault%" == "1" (
    if "%_runT262%" == "1" (
        set _t262OnlyMode=1
    )
)

set path=%path%;%_binaryRoot%;%_toolsRoot%

del /s /q profile.dpl.* > nul

set _JCBinaryArgument=
set _RLMode=-exe

set _Error=0

:: Always turn on FRETEST
set FRETEST=1

::
:: If running on chk or FRETEST, use dynamic profile cache
::
set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -BaselineMode
set _dynamicprofilecache=-dynamicprofilecache:profile.dpl
set _dynamicprofileinput=-dynamicprofileinput:profile.dpl
if "%_buildType%" == "fre" (
    if "%FRETEST%" == "" (
        set _dynamicprofilecache=
        set _dynamicprofileinput=
    )
)
if "%_buildType%" == "chk" (
set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -DumpOnCrash
)

@rem For Apollo, we will exclude tests that have the tag "exclude_apollo".
set _ExcludeApolloTests=
if "%APOLLO%" == "1" (
    set _ExcludeApolloTests=-nottags exclude_apollo
    set TARGET_OS=wp8
)

:: If running as part of DRT/nightly, we do not need to setup the test harnesses.
if "%_drt%" == "" (
    if "%_nightly%" == "" (
        @rem Disable html tests on dev box / snap build machine. Only run in DRT/nightly.
        set _ExcludeHtmlTests=-nottags html

        call runjs setupTesthost

        call runjs setupJdTest

        call runjs setupJsHostTest

        set _ExcludeIntlTests=-nottags exclude_winglob
        call runjs setupWindowsGlobalization
        if errorlevel 0 (
            set _ExcludeIntlTests=
        )
    )
)

:: If we are not running as part of nightly, disable the nightly-only tests
if "%_nightly%" == "" (
    set _NOTTAGS=%_NOTTAGS% -nottags nightly
)

set _BuildFlavorLogs=%_logsRoot%\%_buildArch%%_buildType%
set _delCmd=del /s /q %_BuildFlavorLogs%\rl*.log
echo %_delCmd%
%_delCmd% 2>&1

if NOT "%_SNAP%" == "1" (
    set _BaseVariants=interpreted,dynapogo
) else (
    set _BaseVariants=interpreted,dynapogo
)


:: if -dirs is passed then by default don't run tests in debugmode unless user provides -rundebug explicitly in arguments
if NOT "%_DIRS%" == "" (
    if NOT "%_RUNDEBUG%" == "1" (
        if "%_Variants%"=="" set _Variants=%_BaseVariants%
    )
)

:: If _Variants is modified make corresponding change in unittest_variants array in file jscript\tools\ScriptLib\EzeAutomation.js
if "%_Variants%"=="" set _Variants=%_BaseVariants%

:: If the user specified extra variants to run, include them.
if NOT "%_ExtraVariants%" == "" set _Variants=%_Variants%,%_ExtraVariants%

if "%_ccChangeList%" == "" (
    goto :Run
)

echo #################Code Coverage setup#################
:: Set up CodeCoverage machinery
set "_DpkFile=ChakraCC_%_ccChangeList%"
if "%_ccChangeList%" == "0" (
    echo Code coverage will be done for entire source
) else (
    echo Code Coverage will be done for the changelist %_ccChangeList%
)

:: Check Gumshoe installation
set _gumshoeInstalled=1
where /q gumshoe || set _gumshoeInstalled=0
if "%_gumshoeInstalled%"=="1" (
    echo Gumshoe is already installed
) else (
    if NOT "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
        set _GumshoeSetupPath=\\ocentral\products\Gumshoe\latest\setup\x86\gumshoe.msi
    )
    echo Installing Gumshoe from %_GumshoeSetupPath%
    msiexec.exe /q /i "%_GumshoeSetupPath%" /log GumshoeSetup.log
)
if EXIST "%_MagellanPath%\Magellan\BBCover.exe" (
    echo Magellan latest bits are already installed
) else (
    mkdir %_MagellanPath%"
    if NOT "%PROCESSOR_ARCHITECTURE%"=="AMD64" (
        set _MagellanSetupPath=\\codecovfs01\Magellan_pre_Release\Latest\x86fre
    )
    echo Copying latest Magellan bits from %_MagellanSetupPath% 
    xcopy /S /C /Q /Y %_MagellanSetupPath% %SystemDrive%\ChakraMagellan
)

if not exist "%_GumshoeExe%" (
    echo WARNING: Gumshoe installation failed
) else (
    :: Stop current sessions if any
    call "%_GumshoeExe%" stop

    :: Copying the PDBs
    xcopy %_NTTREE%\Symbols.pri\jscript\dll\chakratest.pdb %_NTTREE%\jscript /C /Y /Q
    if exist "%_NTTREE%\Symbols.pri\jscript\dll\chakralstest.pdb" (
        xcopy %_NTTREE%\Symbols.pri\jscript\dll\chakralstest.pdb %_NTTREE%\jscript /C /Y /Q
    )

    :: Instrumenting the binaries
    %_MagellanPath%\Magellan\BBCover.exe /block /CovSym /I %_NTTREE%\jscript\chakratest.dll /Cmd %~dp0..\tools\CodeCoverage\chakratest.dll.bbtf
    move /Y %_NTTREE%\jscript\chakratest.dll %_NTTREE%\jscript\chakratest.dll.old
    move /Y %_NTTREE%\jscript\chakratest.dll.block.instr %_NTTREE%\jscript\chakratest.dll
    if exist "%_NTTREE%\jscript\chakralstest.dll" (
        %_MagellanPath%\Magellan\BBCover.exe /block /CovSym /I %_NTTREE%\jscript\chakralstest.dll
        move /Y %_NTTREE%\jscript\chakralstest.dll %_NTTREE%\jscript\chakralstest.dll.old
        move /Y %_NTTREE%\jscript\chakralstest.dll.block.instr %_NTTREE%\jscript\chakralstest.dll
    )

    :: Starting the Code Coverage session
    call "%_GumshoeExe%" start
    call "%_GumshoeExe%" add covsym:%_NTTREE%\jscript\chakratest.dll.covsym
    if exist "%_NTTREE%\jscript\chakralstest.dll.covsym" (
        call "%_GumshoeExe%" add covsym:%_NTTREE%\jscript\chakralstest.dll.covsym
    )
    if not "%_ccChangeList%"=="0" (
        :: Create DPK for the change which will be added to the Gumshoe session
        call sdp pack %_DpkFile% -c %_ccChangeList%
        call "%_GumshoeExe%" add dpk:%_DpkFile%.dpk
    )
    call "%_GumshoeExe%" status
)
echo #################Code Coverage setup done#################

:Run
if not "%_t262OnlyMode%" == "1" (
    for %%i in (%_Variants%) do (
        set _TESTCONFIG=%%i
        call :RunOneConfig
    )
)

if "%_ccChangeList%"=="" (
    goto :RunT262
)
if exist "%_GumshoeExe%" (
    echo #################Code Coverage cleanup#################
    :: Stop the session
    call "%_GumshoeExe%" stop
    echo Look at the results at C:\ProgramData\Gumshoe\*.gumproj

    :: Reverting back CodeCoverage binary changes
    move /Y %_NTTREE%\jscript\chakratest.dll %_NTTREE%\jscript\chakratest.dll.block.instr
    move /Y %_NTTREE%\jscript\chakratest.dll.old %_NTTREE%\jscript\chakratest.dll
    if exist "%_NTTREE%\jscript\chakralstest.dll" (
        move /Y %_NTTREE%\jscript\chakralstest.dll %_NTTREE%\jscript\chakralstest.dll.block.instr
        move /Y %_NTTREE%\jscript\chakralstest.dll.old %_NTTREE%\jscript\chakralstest.dll
    )
    echo #################Code Coverage cleanup done#################
)

:RunT262
if "%_runT262%"=="1" (
    echo #################Starting Test262 tests#################
    if not exist %_BuildFlavorLogs% (
        mkdir %_BuildFlavorLogs%
    )
    powershell %SDXROOT%\onecoreuap\inetcore\jscript\tools\Test262\RunTest262Test.ps1 -b %_NTTREE%\jscript\jshost.exe -c "-es6all" -s 75 -f %_BuildType% -l %SDXROOT%\onecoreuap\inetcore\jscript\tools\Test262 -o %_BuildFlavorLogs%\Test262Results.log
    echo #################Done with Test262 tests#################
)
echo.
echo.

if not "%_t262OnlyMode%" == "1" (
    for %%i in (%_Variants%) do (
        echo ######## Logs for %%i variant ########
        type %_BuildFlavorLogs%\%%i\rl.log
        echo.
    )
)

exit /b %_Error%

:::::::::::::::::::::::::::::::::::::::::::::::::
:RunOneConfig
::
:: IMPORTANT: this subroutine returns pass/fail in the _Error environment
:: variable.  Don't surround this call with setlocal/endlocal, or changes
:: to that variable will not be persisted, and failures will not be reflected
:: to SNAP.
::

echo ############# Starting %_TESTCONFIG% variant #############

set _delCmd=del /q %_logsRoot%\rl*
echo %_delCmd%
%_delCmd% 2>&1

set _mdCmd=md %_BuildFlavorLogs%\%_TESTCONFIG%
echo %_mdCmd%
%_mdCmd% 2>&1

set _OLD_CC_FLAGS=%EXTRA_CC_FLAGS%
set EXTRA_RL_FLAGS=
if "%_TESTCONFIG%"=="interpreted" (
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -maxInterpretCount:1 -maxSimpleJitRunCount:1 -bgjit- %_dynamicprofilecache%
    set EXTRA_RL_FLAGS=-appendtestnametoextraccflags
)
if "%_TESTCONFIG%"=="nonative" (
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -nonative
    set EXTRA_RL_FLAGS=-nottags exclude_interpreted -nottags fails_interpreted
)
if "%_TESTCONFIG%"=="dynapogo"    (
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceNative -off:simpleJit -bgJitDelay:0 %_dynamicprofileinput%
    set EXTRA_RL_FLAGS=-appendtestnametoextraccflags
)

set _exclude_serialized=

:: Variants after here are user supplied variants (do not run by default).
if "%_TESTCONFIG%"=="forcedeferparse" (
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceDeferParse %_dynamicprofilecache%
    set EXTRA_RL_FLAGS=-appendtestnametoextraccflags
    set _exclude_forcedeferparse=-nottags exclude_forcedeferparse
)
if "%_TESTCONFIG%"=="nodeferparse" (
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -noDeferParse %_dynamicprofilecache%
    set EXTRA_RL_FLAGS=-appendtestnametoextraccflags
    set _exclude_nodeferparse=-nottags exclude_nodeferparse
)
if "%_TESTCONFIG%"=="forceundodefer" (
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceUndoDefer %_dynamicprofilecache%
    set EXTRA_RL_FLAGS=-appendtestnametoextraccflags
    set _exclude_forceundodefer=-nottags exclude_forceundodefer
)
if "%_TESTCONFIG%"=="bytecodeserialized" (
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -recreatebytecodefile -serialized:%TEMP%\ByteCode
    set EXTRA_RL_FLAGS=-appendtestnametoextraccflags
    set _exclude_serialized=-nottags exclude_serialized
)
if "%_TESTCONFIG%"=="forceserialized" (
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -forceserialized
    set _exclude_serialized=-nottags exclude_serialized
)

echo %_TESTCONFIG% > %_logsRoot%\_currentRun.tmp
:: Default variant is no longer run.
:: if "%_TESTCONFIG%"=="default"      set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -speculationcap:0 %_dynamicprofilecache%
set _runCmd=call %_runAllRLTestsDir%\runtests.cmd %_JCBinaryArgument% %_DIRS% -logverbose %_TAGS% %_NOTTAGS% %_DIRTAGS% %_DIRNOTTAGS% -nottags fails_%_TESTCONFIG% -nottags fail_%TARGET_OS% -nottags exclude_%_TESTCONFIG% -nottags exclude_%_buildArch% -nottags exclude_%TARGET_OS% -nottags exclude_%_buildType% %_exclude_serialized% %_exclude_forcedeferparse% %_exclude_nodeferparse% %_exclude_forceundodefer% %_ExcludeHtmlTests% %_ExcludeIntlTests% %_ExcludeApolloTests% %_RLMode% %EXTRA_RL_FLAGS% %_rebase%
echo %_runCmd%
%_runCmd% 2>&1

set EXTRA_CC_FLAGS=%_OLD_CC_FLAGS%

if errorlevel 1 set _Error=1
if not errorlevel 0 set _Error=1

if _Error==0 del /Y %TEMP%\ByteCode*.bc
if _Error==0 del /Y %TEMP%\NativeCode*.dll

set _moveCmd=move /Y %_logsRoot%\*.log %_BuildFlavorLogs%\%_TESTCONFIG%
echo %_moveCmd%
%_moveCmd% 2>&1

del /Q %_logsRoot%\_currentRun.tmp

@if not defined _echo echo off

:::::::::::::::::::::::::::::::::::::::::::::::::::::
:: This script installs unit tests and binaries to ARM/WOA (or any other Win8) machine
:: - Directions:
::   1) Run this script from razzle giving it ARM/WOA machine name on command line, like this:
::      arm_InstallTests.cmd ezea1
::   2) After this finishes, go to the ARM machine and run:
::      C:\UnitTests\RunAllTests.cmd
:: - Details:
::   The directory structure becomes like this:
::     C:\UnitTests\
::       jscript\  - binary files: chakra.dll, jshost.exe, etc
::       UnitTest\ - copy (mirror mode) of the inetcore\jscript directory
::       RunAllTests.cmd

setlocal ENABLEEXTENSIONS
set Destination=
set DestMachine=
set DestDrive=C
set DestDirName=UnitTests
set RunScriptName=RunAllTests.cmd
set SetenvScriptName=setenv.cmd
set NoPdb=
set NoTests=
set NoBinaries=
set IsWin8=0
set Called=%0

set PDMRoot=%SDXROOT%\inetcore\devtoolbar\jstools\setup\%_BuildArch%
set DiffBin=\\bptstorage3\users\tcare\diff\%_BuildArch%\diff.exe

call :CheckUnderRazzle || exit /b %errorlevel%

call :ParseArgs %* || exit /b %errorlevel%

call :RunInstall || exit /b %errorlevel%
goto :eof

::::::::::::::::::::::::::::::::::::::::::::::::::::
:: Subroutines
:RoboCopyWrapper
    REM Avoid errorlevel weirdness between calls.
    set errorlevel=
    robocopy %*
    SET errlevel=%errorlevel%
    call :CheckRobocopyErrorLevel %errlevel% || exit /b %errlevel%
    goto :eof

:CheckRoboCopyErrorLevel
    if %1 GEQ 8 (
        echo :: ERROR: call :RoboCopyWrapper failed to copy files. Exiting.
        exit /b %1
    )
    if %1 EQU 4 (
        echo :: WARNING: call :RoboCopyWrapper reported mismatched files. Consider cleaning the destination directory.
        exit /b %1
    )
    goto :eof

:CheckXCopyErrorLevel
    if %errorlevel% GEQ 4 (
        echo :: ERROR: XCopy failed; aborting.
        exit /b %errorlevel%
    ) else if %errorlevel% EQU 2 (
        echo :: ABORT: User cancelled, leaving script.
        exit /b %errorlevel%
    ) else if %errorlevel% EQU 1 (
        echo :: ERROR: Could not find the source file. The script may need updating.
        exit /b %errorlevel%
    )
    goto :eof

:CheckNetUseErrorLevel
    if %errorlevel% GTR 0 (
        echo :: ERROR: Could not access share; check your credentials, permissions, that the destination share exists and file sharing is enabled. Aborting.
        exit /b %errorlevel%
    )
    goto :eof

:ParseArgs
    if "%1" == "" (
        call :PrintUsage
        exit /b 1
    )
REM Fall-through intentional
:NextArgument
    if /i "%1" == "-nopdb" (
        set NoPdb=1
        goto ArgLoop
    ) else if /i "%1" == "-nobin" (
        set NoBinaries=1
        goto ArgLoop
    ) else if /i "%1" == "-notest" (
        set NoTests=1
        goto ArgLoop
    ) else if "%1" == "-win8" (
        set IsWin8=1
        goto ArgLoop
    ) else if "%1" == "" (
        goto :eof
    ) else (
        call :GetDestination %1 || exit /b %errorlevel%
        goto ArgLoop
    )
:ArgLoop
    shift
    goto :NextArgument
    goto :eof

:RunInstall
    call :MkDirIfNotExist %Destination%         || exit /b %errorlevel%
    call :MkDirIfNotExist %Destination%\jscript || exit /b %errorlevel%
    call :MkDirIfNotExist %Destination%\dl      || exit /b %errorlevel%

    if "%NoBinaries%" == "" call :CopyBinaries || exit /b %errorlevel%
    if "%NoPdb%" == ""      call :CopyPdbs     || exit /b %errorlevel%
    if "%NoTests%" == "" (
        call :CopyUnitTestsDir || exit /b %errorlevel%
        REM TODO: move all test dependencies under jscript\unittest and remove the call below.
        call :CopyTestDependencies || exit /b %errorlevel%
    )
    call :CreateScriptRunAllTests && echo Now remote-desktop to %DestMachine% and run %DestDrive%:\%DestDirName%\%RunScriptName% there.

    goto :eof

:CopyBinaries
    REM The idea is to keep exactly same directory structure as on the build/enlistment machine.
    if "%_BuildArch%" == "arm" || "%_BuildArch%" == "arm64"(
       echo :: NTSigning JScript9 binaries ^(required for debugger loaded dlls on arm^)...
       for %%i in (%_NTTREE%\chakra*.dll %_NTTREE%\jscript\*.dll %_NTTREE%\jscript\*.exe) do (
           echo NTSigning %%i 
           call ntsign %%i >NUL || exit /b %errorlevel%
       )
       echo.
    )
    
    echo :: Copying JScript9 binaries...
    call :RoboCopyWrapper %_NTTREE%         %Destination%         chakra*.dll /njh /njs /ndl /purge /xx || exit /b %errorlevel%
    call :RoboCopyWrapper %_NTTREE%\jscript %Destination%\jscript *.dll *.exe   /njh /njs /ndl /purge /xx /xf pdm.dll pdmproxy100.dll msdbg2.dll || exit /b %errorlevel%
    echo.
    REM echo :: Copying debugger support binaries...
    call :RoboCopyWrapper %PDMRoot%         %Destination%         *.dll         /njh /njs /ndl /purge /xx || exit /b %errorlevel%
    call :RoboCopyWrapper %PDMRoot%         %Destination%\jscript *.dll         /njh /njs /ndl /purge /xx || exit /b %errorlevel%
    echo.
    if %IsWin8% NEQ 0 (
        echo :: Copying %_BuildArch% downlevel globalization dll ^(credentials may be required^)...
        net use \\iefs\IEFILES\Tools\jscript\WindowsGlobalization\%_BuildArch%fre
        call :CheckNetUseErrorLevel || exit /b %errorlevel%
        xcopy /y \\iefs\IEFILES\Tools\jscript\WindowsGlobalization\%_BuildArch%fre\Windows.Globalization.dll %_NTTREE%\jscript\
        call :CheckXCopyErrorLevel  || exit /b %errorlevel%
        echo.
    )
    goto :eof

:CopyPdbs
    REM The idea is to keep each pdb next to the appropriate binary
    echo :: Copying JScript9 PDBs...
    call :RoboCopyWrapper %_NTTREE%\symbols.pri\jscript\   %Destination%\jscript *.pdb         /njh /njs /ndl /purge /xx /xf pdm.pdb pdmproxy100.pdb msdbg2.pdb || exit /b %errorlevel%
    call :RoboCopyWrapper %_NTTREE%\symbols.pri\retail\dll %Destination%         chakra*.pdb /njh /njs /ndl /purge /xx || exit /b %errorlevel%
    echo.
    echo :: Copying debugger support PDBs...
    call :RoboCopyWrapper %PDMRoot%                        %Destination%         *.pdb         /njh /njs /ndl /purge /xx || exit /b %errorlevel%
    call :RoboCopyWrapper %PDMRoot%                        %Destination%\jscript *.pdb         /njh /njs /ndl /purge /xx || exit /b %errorlevel%
    echo.
    goto :eof

:CopyUnitTestsDir
    REM Note: call :RoboCopyWrapper /mir copies in mirror mode: this removes all files in destination that are not in source.
    echo :: Copying unit tests... 
    call :RoboCopyWrapper %SDXROOT%\inetcore\jscript\unittest "%Destination%\unittest" /nfl /mir /xd logs ES5Conformance VSTests /xf testout* *.dpl* || exit /b %errorlevel%
    echo.
    goto :eof

:CopyTestDependencies
    REM Note: call :RoboCopyWrapper /mir copies in mirror mode: this removes all files in destination that are not in source.
    echo :: Copying unit test support files...
    xcopy /y %DiffBin% %Destination%\jscript\
    call :CheckXCopyErrorLevel || exit /b %errorlevel%
    call :RoboCopyWrapper %SDXROOT%\inetcore\jscript\Lib\Author\References              "%Destination%\Lib\Author\References"              /mir /njh /njs || exit /b %errorlevel%
    call :RoboCopyWrapper %SDXROOT%\inetcore\jscript\Lib\Runtime\Library\InJavascript   "%Destination%\Lib\Runtime\Library\InJavascript"   /mir /njh /njs || exit /b %errorlevel%
    call :RoboCopyWrapper %SDXROOT%\inetcore\jscript\Lib\Common\Memory\ValidPointersMap "%Destination%\Lib\Common\Memory\ValidPointersMap" /mir /njh /njs || exit /b %errorlevel%
    echo.
    goto :eof

:CreateScriptRunAllTests
    REM Notes:
    REM   - _BuildArch is needed for runtest.cmd
    REM   - _NTTREE is needed for jsglass.exe
    REM   - The reason to use setenv.cmd: if you want to run test manually, run setenv.cmd to prepare the environment.
    set SetenvFile=%Destination%\%SetenvScriptName%
    set ScriptFile=%Destination%\%RunScriptName%
    (
        echo REM This file was created by script %~f0 ran on build/enlistment machine.
        echo set _NTTREE=%%~dp0
        echo set _BuildArch=%_BuildArch%
        echo set build.arch=%build.arch%
        echo set build.type=%build.type%
        echo set JSUnitTestDir=%%_NTTREE%%unittest
        echo set JSCRIPT_ROOT=%%_NTTREE%%
        echo set PATH=%%_NTTREE%%jscript;%%path%%
        echo set _NT_SYMBOL_PATH=%%_NT_SYMBOL_PATH%%;%%_NTTREE%%;%%_NTTREE%%jscript
    ) > %SetenvFile%
    (
        echo REM This file was created by script %~f0 ran on build/enlistment machine.
        echo REM Note: in order to run only native ^(or interpreted^), set _Variants=native ^(or interpreted^) before running this script.
        echo setlocal
        echo call %%~dp0%SetenvScriptName%
        echo pushd %%JSUnitTestDir%%
        echo call RunAllRLTests.cmd %%*
        echo popd
        echo endlocal
    ) > %ScriptFile% && echo Created %ScriptFile%.
    goto :eof

:CheckUnderRazzle
    if "%SDXROOT%" == "" (
        echo :: ERROR: Razzle has not been detected. You must run this from a Razzle environment.
        call :PrintUsage || exit /b %errorlevel%
        exit /b 3
    )
    goto :eof

:MkDirIfNotExist
    if not exist %~f1 (
        echo md %~f1
        md %~f1
    )
    if not exist %~f1 (
        echo :: ERROR: Could not create destination directory %~f1. Check your permissions. Aborting.
        exit /b 2
    )
    goto :eof

:GetDestination
    set Dest=%1
    if "%Dest%" == "" (
        echo Error: machine name, local path, or remote path is missing
        call :PrintUsage
        exit /b 1
    )
    REM Determine if this is a network path.
    if defined Dest if "%Dest:~0,2%"=="\\" (
        REM Assume remote path.
        echo :: Attempting to connect to remote path %Dest%...
        net use %Dest%
        call :CheckNetUseErrorLevel || exit /b %errorlevel%
        set Destination=%Dest%
    ) else if exist %Dest% (
        REM Assume local path.
        echo :: Using local path %Dest%.
        set Destination=%Dest%
    ) else (
        REM Assume machine name.
        set DestRoot=\\%Dest%\%DestDrive%$
        echo %DestRoot%
        set Destination=%DestRoot%\%DestDirName%
        echo %Destination%
        echo :: Attempting to connect to %DestRoot%...
        net use %DestRoot%
        call :CheckNetUseErrorLevel || exit /b %errorlevel%
    )
    goto :eof

:PrintUsage
    echo %~nx0: 
    echo   Deploys binaries, pdbs, unit tests and environment to run unit tests on another machine.
    echo.
    echo Usage: %Called% [-nopdb] [-nobin] [-notest] [-win8] ^<machine name^>^|^<local path^>^|^<\\remote\path^>
    echo   -nopdb ^(optional^):  do not copy the pdbs, this speeds up an initial copy
    echo   -nobin ^(optional^):  do not copy the binaries
    echo   -notest ^(optional^): do not copy the unittest dir, this speeds up copy a lot
    echo   -win8: target deployment for win8 (copies downlevel dlls)
    echo.
    echo   Destinations:
    echo   ^<machine name^>:  unit tests will be installed to \\^<machine name^>\%DestDrive%$\%DestDirName% (directory will be created)
    echo   ^<local path^>:    unit tests will be installed to the local path (directory must exist)
    echo   ^<\\remote\path^>: unit tests will be installed to the remote path (directory must exist)
    echo.
    echo   After the script finishes, run RunAllTests.cmd ^<flags^> at the destination.
    goto :eof

endlocal

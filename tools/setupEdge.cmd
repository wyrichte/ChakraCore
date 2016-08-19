@echo off
setlocal

REM ---------------------
REM Init
REM ---------------------
set WINBUILD=\\winbuilds\release\
set BRANCH_PREFIX=rs_onecore_webplat_stage
set WINBUILDBASE=
set BUILDVER=
set COPYARCH=
set COPYTARGETARCH=

REM ---------------------
REM ParseArg
REM ---------------------
:ParseArg
IF "%1" == "" (
    goto :DoneParse
)

IF "%1" == "-?" (
    goto :Usage
)

IF /i "%1" == "x86" (
    set COPYTARGETARCH=%1
    goto :ParseOne
)

IF /i "%1" == "x64" (
    set COPYTARGETARCH=%1
    goto :ParseOne
)

IF /i "%1" == "amd64" (
    set COPYTARGETARCH=x64
    goto :ParseOne
)

IF EXIST "%_DEST%\" (
    set _DEST=%1
    goto :ParseOne
)

IF NOT "%BUILDVER%" == "" (
    goto :SkipParseArgBuildVer
)

IF NOT "%WINBUILDBASE%" == "" (
    goto :SkipParseArgWinBuildBase
)

IF EXIST "%WINBUILD%\%BRANCH_PREFIX%_%1" (
    set WINBUILDBASE=%WINBUILD%\%BRANCH_PREFIX%_%1
    goto :ParseOne
)

IF EXIST "%WINBUILD%\%1" (
    set WINBUILDBASE=%WINBUILD%\%1
    goto :ParseOne
)

:SkipParseArgWinBuildBase

FOR /F "usebackq" %%i IN (`dir/b/on %WINBUILDBASE%\*%1*`) DO ( 
    set BUILDVER=%%i
)

IF NOT "%BUILDVER%" == "" (
    goto :ParseOne
)

:SkipParseArgBuildVer

echo ERROR: Invalid argument %1
goto :Usage

:ParseOne
shift /1
goto :ParseArg

:DoneParse

REM ---------------------
REM Defaults
REM ---------------------
IF "%COPYTARGETARCH%" == "" (
    IF NOT "%ChakraBuildArch%" == "" (
        set COPYTARGETARCH=%ChakraBuildArch%
    ) ELSE (
        set COPYTARGETARCH=%_BuildArch%
    )
)
IF "%COPYTARGETARCH%" == "x86" (
    set COPYARCH=x86
) ELSE IF "%COPYTARGETARCH%" == "x64" (
    set COPYARCH=amd64
) ELSE (
    echo ERROR: Invalid arch %COPYTARGETARCH%
)

IF "%WINBUILDBASE%" == "" (
    set WINBUILDBASE=\\winbuilds\release\RS_ONECORE_WEBPLAT_STAGE_DEV3
)

IF NOT "%BUILDVER%" == "" (
    goto :SkipDefaultBuildVer
)

FOR /F "usebackq" %%i IN (`dir/b/o-n %WINBUILDBASE%`) DO (
    IF EXIST "%WINBUILDBASE%\%%i\%COPYARCH%fre\bin" (
        set BUILDVER=%%i
        goto :ExitBuildVerSearch
    )
)

:ExitBuildVerSearch

IF "%BUILDVER%" == "" (
    echo ERROR: No build in %WINBUILDBASE%
)

:SkipDefaultBuildVer
IF "%_DEST%" == "" (
    IF NOT "%ChakraFullBinDir%" == "" (
        set _DEST=%ChakraFullBinDir%
    ) ELSE IF NOT "%OutBaseDir%" == "" (
        IF NOT "%ChakraBuildType%" == "" (
            set _DEST=%OutBaseDir%\Chakra.Full\bin\%COPYTARGETARCH%_%ChakraBuildType%
        ) ELSE (
            set _DEST=%OutBaseDir%\Chakra.Full\bin\%COPYTARGETARCH%_%_BuildType%
        )
    ) ELSE (
        echo ERROR: Unknown destination
    )
)


IF NOT EXIST "%_DEST%\" (
    echo ERROR: Directory %_DEST% does not exist
)

call :EnsureDirectory "%_DEST%\jshost.exe.local" 
call :EnsureDirectory "%_DEST%\jshost.exe.local\en-us" 
call :EnsureDirectory "%_DEST%\mshtmpad.exe.local" 
call :EnsureDirectory "%_DEST%\mshtmpad.exe.local\en-us" 
call :EnsureDirectory "%_DEST%\wpaxhost.exe.local" 
call :EnsureDirectory "%_DEST%\wpaxhost.exe.local\en-us" 

set BINSRC=%WINBUILDBASE%\%BUILDVER%\%_BuildArch%fre\bin
set SYMSRC=%WINBUILDBASE%\%BUILDVER%\%_BuildArch%fre\symbols.pri
set MUISRC=%WINBUILDBASE%\%BUILDVER%\%_BuildArch%fre\en-us\bin_segmented\onecoreuap

echo FROM : %BINSRC%
echo TO   : %_DEST%\jshost.exe.local
echo .

REM Core set
call :CopyBinFile edgehtml.dll
call :CopyBinFile ieproxy.dll
call :CopyBinFile iertutil.dll
call :CopyBinFile urlmon.dll
call :CopyBinFile wininet.dll

REM Debug related
call :CopyBinFile msdbg2.dll
call :CopyBinFile pdm.dll
call :CopyBinFile pdmproxy100.dll


REM MSHTMLPAD Only
call :CopyBinFilePadOnly mshtmdbg.dll
REM REVIEW: Don't know where the pdb is
call :CopyFile %BINSRC%\mshtmdbgshim.dll %_DEST%\mshtmpad.exe.local
call :CopyBinFilePadOnly DiagnosticsTap.dll
call :CopyBinFilePadOnly F12App.dll
call :CopyBinFilePadOnly F12AppFrame.dll
call :CopyBinFilePadOnly F12Platform.dll
call :CopyBinFilePadOnly F12Script.dll
call :CopyBinFileExPadOnly Spartan\omega.dll Spartan 
call :CopyBinFileExPadOnly Spartan\htmlpad.dll Spartan 
call :CopyBinFileExPadOnly TouchInjector\TouchInjector.dll TouchInjector
call :CopyBinFileEx Spartan\mshtmpad.exe Spartan %_DEST%

call :CopyBinFileEx wpaxhost.exe retail %_DEST%
call :CopyBinFileEx wpaxholder.dll retail %_DEST%\wpaxhost.exe.local
call :CopyBinFileEx mshtmdbg.dll retail %_DEST%\wpaxhost.exe.local

exit /B 0

REM ------------------
REM Copy Utilities
REM ------------------
:CopyBinFile
call :CopyBinFileEx %1 retail 
exit /B 0

:CopyBinFilePadOnly
call :CopyBinFileEx %1 retail %_DEST%\mshtmpad.exe.local
exit /B 0

:CopyBinFileExPadOnly
call :CopyBinFileEx %1 %2 %_DEST%\mshtmpad.exe.local
exit /B 0


:CopyBinFileEx
echo Copying %1
call :CopyFile %BINSRC%\%1 %3
IF "%~x1" == ".dll" (
    call :CopyFile %SYMSRC%\%2\dll\%~n1.pdb %3
) ELSE (
    call :CopyFile %SYMSRC%\%2\exe\%~n1.pdb %3
)
IF EXIST "%MUISRC%\%1.mui" (
    call :CopyFileEx \en-us\ %MUISRC%\%1.mui 
)
exit /B 0

:CopyFile
call :CopyFileEx \ %1 %2
exit /B 0

:CopyFileEx
set _DESTDIR=%3
IF "%_DESTDIR%" == "" (
    set _DESTDIR=%_DEST%\jshost.exe.local%1
)
xcopy /y %2 %_DESTDIR% > nul
IF NOT "%ERRORLEVEL%" == "0" (
    echo ERROR: Failed to copy %2 to %_DESTDIR%
)
IF "%3" == "" (
    xcopy /y %_DESTDIR%%~nx2 %_DEST%\mshtmpad.exe.local%1 > nul
)
exit /B 0

:EnsureDirectory
IF NOT EXIST "%1\" (
    mkdir %1
)
exit /B 0


:Usage
echo setupEdge.cmd [^<branch^>] [^<build^>] [^<dest^>]
echo     ^<branch^> - Full branch name, or suffix to rs_onecore_webplat_stage_^<branch^> 
echo                (e.g. rs_onecore_webplat_stage_dev3 or dev3)
echo                Default: rs_onecore_webplat_stage_dev3
echo .
echo     ^<build^>  - Build number, matches the newest directory with *^<build^>* in the build number
echo                (e.g. 14906.1000.160816-2000, or 0816)
echo                Default: latest
echo .
echo     ^<dest^>   - Destination directory
echo                Default: %%ChakraFullBinDir%%
exit /B 0

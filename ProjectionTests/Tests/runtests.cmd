@setlocal
@echo off

set _JCBinaryArgument=-binary:%SDXROOT%\inetcore\jscript\win8unittest\JsHost\JsHost.exe
set _ArgTags=
set _Dirs=-all
set _OutputArgument=
set _OLD_CC=%EXTRA_CC_FLAGS%

REM set defaults for all tests - TargetWinRTVersion:3 is BLUE
set EXTRA_CC_FLAGS=-bvt -hosttype:2 -TargetWinRTVersion:3 %EXTRA_CC_FLAGS%

:NextArgument
if /i "%1" == "-verbose" (
    echo on
    goto ArgLoop
) else if "%1" == "-?" (
    echo Usage: runtests.cmd [-verbose] [-tags tag1,...] [-nottags tag1,...] [-binary ^<path^>]
    exit /b 0
) else if /i "%1" == "-tags" (
    set _ArgTags=%_ArgTags% -tags:%2
    shift
    goto ArgLoop
) else if /i "%1" == "-nottags" (
    set _ArgTags=%_ArgTags% -nottags:%2
    shift
    goto ArgLoop
) else if /i "%1" == "-binary" (
    set _JCBinaryArgument=-binary:%2
    shift
    goto ArgLoop
) else if /i "%1" == "-dirs" (
    set _Dirs=-dirs:%2
    shift
    goto ArgLoop
) else if /i "%1" == "-exe" (
    set _RLMode=-exe
    goto ArgLoop
) else if /i "%1" == "-asmbase" (
    set _RLMode=-asm -base
    set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -Off:InsertNOPs -Prejit
    goto ArgLoop
) else if /i "%1" == "-asmdiff" (
    set _RLMode=-asm -diff
        set EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS% -Off:InsertNOPs -Prejit
    goto ArgLoop
) else if /i "%1" == "-logverbose" (
    set _OutputArgument=-verbose
    goto ArgLoop
) else if "%1" == "" (
    goto StartScript
)

:ArgLoop
shift
goto :NextArgument


:StartScript

call setenv.cmd
if errorlevel 1 exit /b 1

if "%_BuildArch%" == "" (
    set _localBuildArch=x86
)else (
    set _localBuildArch=%_BuildArch%
)

if "%NUMBER_OF_PROCESSORS%" == "12" (
    set RLCMD=rl -threads:1 -target:%_localBuildArch% %_JCBinaryArgument% -nottags:fail,es5 -nottags:exclude_%_localBuildArch% %_ArgTags% %_RLMode% %_Dirs% %_OutputArgument%
) else (
    set RLCMD=rl -target:%_localBuildArch% %_JCBinaryArgument% -nottags:fail,es5 -nottags:exclude_%_localBuildArch% %_ArgTags% %_RLMode% %_Dirs% %_OutputArgument%
)
echo EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS%
echo %CD%: %RLCMD%
%RLCMD%

if errorlevel 1 set _ERROR=1
if not errorlevel 0 set _ERROR=1

set EXTRA_CC_FLAGS=%_OLD_CC%

exit /b %_ERROR%

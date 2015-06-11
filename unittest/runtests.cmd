@setlocal
@echo off

set _JCBinaryArgument=
set _ArgTags=
set _Dirs=-all
set _OutputArgument=
set _OLD_CC=%EXTRA_CC_FLAGS%
set _rebase=

:NextArgument
if /i "%1" == "-verbose" (
    echo on
    goto ArgLoop
)
if "%1" == "-?" (
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
) else if /i "%1" == "-dirtags" (
    set _ArgTags=%_ArgTags% -dirtags:%2
    shift
    goto ArgLoop
) else if /i "%1" == "-dirnottags" (
    set _ArgTags=%_ArgTags% -dirnottags:%2
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
) else if /i "%1" == "-rebase" (
    set _rebase=-rebase
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
    set _OutputArgument=%_OutputArgument% -verbose
    goto ArgLoop
) else if /i "%1" == "-appendtestnametoextraccflags" (
    set appendtestnametoextraccflags=-appendtestnametoextraccflags
) else if "%1" == "" (
    goto StartScript
)

:ArgLoop
shift
goto :NextArgument


:StartScript

call setenv.cmd
if errorlevel 1 exit /b 1

if "%NUM_RL_THREADS%" NEQ "" (
    set _RL_THREAD_FLAGS=-threads:%NUM_RL_THREADS%
)

set RLCMD=rl -target:%_BuildArch% %_JCBinaryArgument% -nottags:fail %_RL_THREAD_FLAGS% %_ArgTags% %_RLMode% %_Dirs% %_OutputArgument% %appendtestnametoextraccflags% %_rebase%
echo EXTRA_CC_FLAGS=%EXTRA_CC_FLAGS%
echo %RLCMD%
%RLCMD%

if errorlevel 1 set _ERROR=1
if not errorlevel 0 set _ERROR=1

set EXTRA_CC_FLAGS=%_OLD_CC%

exit /b %_ERROR%

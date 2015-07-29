@echo off

setlocal

if "%_LoggingParams%" EQU "" (
    set _LoggingParams=/fl1 /flp1:logfile=build_%_BuildArch%%_BuildType%.log;verbosity=normal /fl2 /flp2:logfile=build_%_BuildArch%%_BuildType%.err;errorsonly /fl3 /flp3:logfile=build_%_BuildArch%%_BuildType%.wrn;warningsonly
)

set "_msbuildArgs="
set "_msbuildProj="

:parseArgs
set _arg=%1
shift

if "%_arg%" EQU "/c" (
    set _targets=/t:Clean,Build
    goto :parseArgs
)

if "%_arg%" EQU "/C" (
    set _targets=/t:Clean,Build
    goto :parseArgs
)

if "%_arg%" NEQ "" (
    set _msbuildArgs=%_msbuildArgs% %_arg%
    goto :parseArgs
)

echo MSBuildArgs are %_msBuildArgs%

echo msbuild %_msBuildArgs% /p:Configuration=all-%_BuildType% /p:Platform=%_BuildArch% %REPO_ROOT%\Build\Chakra.Full.sln %_msbuildProj% %_LoggingParams% %_targets%

msbuild %_msBuildArgs% /p:Configuration=all-%_BuildType% /p:Platform=%_BuildArch% %REPO_ROOT%\Build\Chakra.Full.sln %_msbuildProj% %_LoggingParams% %_targets%

endlocal

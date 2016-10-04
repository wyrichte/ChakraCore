
@echo off

setlocal

if "%_LoggingParams%" EQU "" (
    set _LoggingParams=/fl1 /flp1:logfile=build_%_BuildArch%%_BuildType%.log;verbosity=normal /fl2 /flp2:logfile=build_%_BuildArch%%_BuildType%.err;errorsonly /fl3 /flp3:logfile=build_%_BuildArch%%_BuildType%.wrn;warningsonly
)

set _ChakraBuildConfig=%_BuildType%
if "%_BuildType%" EQU "chk" (
    set _ChakraBuildConfig=Debug
) else if "%_BuildType%" EQU "fre" (
    set _ChakraBuildConfig=Release
) else if "%_BuildType%" EQU "test" (
    set _ChakraBuildConfig=Test
)

set "_msbuildArgs="
set "_msbuildProj="
set _ChakraSolution=%REPO_ROOT%\Build\Chakra.Full.sln
set _ChakraConfiguration=all
set _CoreBuild=0
:parseArgs
set _arg=%1
shift

if "%_arg%" EQU "/core" (
    set _ChakraSolution=%REPO_ROOT%\core\Build\Chakra.Core.sln
    set _CoreBuild=1
    goto :parseArgs
)

if "%_arg%" EQU "/prefast" (
    set _msbuildArgs=%_msbuildArgs% /p:RunCodeAnalysis=true;CodeAnalysisTreatWarningsAsErrors=true
    goto :parseArgs
)

if "%_arg%" EQU "/c" (
    set _targets=/t:Clean,Build
    goto :parseArgs
)

if "%_arg%" EQU "/C" (
    set _targets=/t:Clean,Build
    goto :parseArgs
)

if "%_arg%" EQU "jshost" (
    set _ChakraConfiguration=jshost
    goto :parseArgs
)

if "%_arg%" NEQ "" (
    set _msbuildArgs=%_msbuildArgs% %_arg%
    goto :parseArgs
)

:DoneParsing

if "%_CoreBuild%" EQU "0" (
    set _ChakraBuildConfig=%_ChakraConfiguration%-%_ChakraBuildConfig%
)

echo MSBuildArgs are %_msBuildArgs%

echo msbuild %_msBuildArgs% /m /p:Configuration=%_ChakraBuildConfig% /p:Platform=%_BuildArch% %_ChakraSolution% %_msbuildProj% %_LoggingParams% %_targets%

msbuild %_msBuildArgs% /m /p:Configuration=%_ChakraBuildConfig% /p:Platform=%_BuildArch% %_ChakraSolution% %_msbuildProj% %_LoggingParams% %_targets%

endlocal

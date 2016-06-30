@echo off
setlocal

set _Error=0
set _drt=
set _snap=
set _binaryRoot=%_nttree%\jscript
set _toolsRoot=%sdxroot%\onecoreuap\inetcore\jscript\tools
set _logFile=
set _unittestRoot=%sdxroot%\onecoreuap\inetcore\jscript\unittest
set _buildType=%build.type%
set _buildArch=%build.arch%

:NextArgument
if /i "%1" == "-drt" (
    set _drt=1
    goto ArgLoop
) else if /i "%1" == "-snap" (
    set _snap=-snap
    goto ArgLoop
) else if /i "%1" == "-binaryRoot" (
    set _binaryRoot=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-toolsRoot" (
    set _toolsRoot=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-logFile" (
    set _logFile=-logFile:"%2"
    shift
    goto ArgLoop
) else if /i "%1" == "-unittestRoot" (
    set _unittestRoot=%2
    shift
    goto ArgLoop
) else if /i "%1" == "-platform" (
    set _buildArch=%2
    shift
    goto :ArgLoop
) else if /i "%1" == "-buildType" (
    set _buildType=%2
    shift
    goto :ArgLoop
) else if "%1" == "" (
    goto StartScript
)

:ArgLoop
shift
goto :NextArgument

:StartScript
set path=%path%;%_binaryRoot%

pushd %_toolsRoot%
call runjs.bat setupWindowsGlobalization
set _callCmd=runjs.bat runJsrtUnitTests %_binaryRoot% %_logFile% %_buildArch% %_buildType% %_snap%
echo %_callCmd%
call %_callCmd% 2>&1
popd

if errorlevel 1 set _Error=1

exit /b %_Error%

:NextArgument
if "%1" == "-?" (
	echo Usage: runalltests.cmd -type ^<buildtype^> -arch ^<buildarch^>
	exit /b 0
)
if "%1" == "-arch" (
    set _BuildArch=%2
    set build.arch=%2
	shift
	goto ArgLoop
)
if "%1" == "-type" (
    set build.type=%2
	shift
	goto ArgLoop
)
if "%1" == "" (
	goto StartScript
)

:ArgLoop
shift
goto :NextArgument


:StartScript
call setenv.cmd
set
rem pause

setlocal
set path=%path%;%_nttree%\bin
set JSUnitTestDir=%_nttree%\browser_inetcore_test_package
set JSProjectionUnitTestDir=%SDXROOT%\inetcore\jscript\ProjectionTests\Tests\

rem Disable projection tests on Win7 since we now run them in SNAP. Probably can delete them entirely, but need to evaluate.
rem set RunProjectionTests=%X86%

pushd %JSUnitTestDir%
call RunAllTestsDontRunThisDirectly.cmd

if not "%RunProjectionTests%" == "1" goto Done

pushd %JSProjectionUnitTestDir%
call runalltests.cmd -win7only
popd
popd

:Done
endlocal




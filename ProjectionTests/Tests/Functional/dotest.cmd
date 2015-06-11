@echo off
SETLOCAL ENABLEEXTENSIONS

rem use jsgen from $_NTTREE%\projection\winrt if available (dev-machine)
rem  otherwise use from the current folder (snap machine)

set _filename=%CD%\temp_jsgen_
set _jsgenArgs=

:NextArgument
if "%1" == "-?" (
	echo Usage: doTest.cmd [-winmd winmdfile] [-rootNamespace namespace] [-r winmdfile] [-enableVersioningAllAssemblies] [-targetPlatformVersion version]
	echo.
	echo OR
	echo Usage: doTest.cmd -call script.cmd
	echo.
	exit /b 0
)
if "%1" == "-call" (
    call %2
    goto :end
)
if "%1" == "-enableVersioningAllAssemblies" (
	set _jsgenArgs=%_jsgenArgs% -enableVersioningAllAssemblies
	set _filename=%_filename%.eva
	goto ArgLoop
)
if "%1" == "-targetPlatformVersion" (
	set _jsgenArgs=%_jsgenArgs% -targetPlatformVersion %2
	set _filename=%_filename%.tPV%2
	shift
	goto ArgLoop
)
if "%1" == "-winmd" (
	set _jsgenArgs=%_jsgenArgs% -winmd %2
	set _filename=%_filename%.winmd%2
	shift
	goto ArgLoop
)
if "%1" == "-rootNamespace" (
	set _jsgenArgs=%_jsgenArgs% -rootNamespace %2
	set _filename=%_filename%.ns%2
	shift
	goto ArgLoop
)
if "%1" == "-r" (
	set _jsgenArgs=%_jsgenArgs% -r %2
	set _filename=%_filename%.r%2
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
set _filename=%_filename%.txt

REM HACK: filter out args - we know from runtests.cmd as 'static' payload
REM Z:\fbl_ie_script_dev\inetcore\jscript\ProjectionTests\Tests\Functional\dotest.cmd -hosttype:2  -NoNative -winmd Fabrikam.Kitchen.winmd -rootNamespace jsgen -r Windows.winmd
REM -- filter out %1, %2, %3

rem echo Running %WIN_JSHOST_METADATA_BASE_PATH%\JsGen.exe %_jsgenArgs% %_filename%
pushd %WIN_JSHOST_METADATA_BASE_PATH%
JsGen.exe %_jsgenArgs% %_filename% >NUL
popd
type %_filename%
del %_filename%

:end
endlocal
goto :eof

:call
call %1
goto :eof
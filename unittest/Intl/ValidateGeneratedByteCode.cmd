@echo off
setlocal

REM If the diff fails below, engine modifications require that the library bytecode be regenerated. 
REM Run %JSCRIPT_ROOT%\lib\runtime\library\InJavaScript\GenByteCode.cmd <bytecodefn>.js
REM Note: this can't be run as post-build validation step as can't run generated just-generated
REM executables in the build (to keep build machines independent of target run architecture)

:NextArgument
if "%1" == "-?" (
	echo "Usage: validategeneratedbytecode.cmd -relbasepath <path> -bytecodefn <fn>"
	exit /b 0
)
if "%1" == "-relbasepath" (
	set relBasePath=%2
	shift
	goto ArgLoop
)
if "%1" == "-bytecodefn" (
	set byteCodeFN=%2
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

IF "%JSCRIPT_ROOT%" == "" (
    set JSCRIPT_ROOT=%sdxroot%\inetcore\jscript
)

set _BASEPATH=%JSCRIPT_ROOT%\%relBasePath%

if "%AMD64%" == "1" (
    set baseFileName=%_BASEPATH%\%byteCodeFN%.js.bc.64b.h
) else (
    set baseFileName=%_BASEPATH%\%byteCodeFN%.js.bc.32b.h
)
set comparisonFile=.\%byteCodeFN%.js.bc.h
IF EXIST %comparisonFile% (
    del %comparisonFile% 
)

%_nttree%\jscript\jshost -GenerateLibraryByteCodeHeader:%comparisonFile% -Intl -Version:5 %_BASEPATH%\%byteCodeFN%.js
if "%errorlevel%" NEQ "0" (
    Echo errorlevel is %errorlevel%
    Echo ValidateByteCode.cmd : error: Error generating bytecode file. Ensure %comparisonFile% is writable. 1>&2
    goto Exit;
)
diff %baseFileName% %comparisonFile%

Echo Pass

del %comparisonFile%

:Exit
exit /b 0

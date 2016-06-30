@echo off
setlocal

:NextArgument
if "%1" == "-?" (
    echo "Usage: %0 -relbasepath <path>"
    exit /b 0
)
if "%1" == "-relbasepath" (
    set relBasePath=%2
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
    set JSCRIPT_ROOT=%sdxroot%\onecoreuap\inetcore\jscript
)

set _BASEPATH=%JSCRIPT_ROOT%\%relBasePath%

if "%AMD64%" == "1" (
    set baseFileName=%_BASEPATH%\vpm.64b.h
) else (
    set baseFileName=%_BASEPATH%\vpm.32b.h
)
set comparisonFile=.\vpm.h
IF EXIST %comparisonFile% (
    del %comparisonFile% 
)

%_nttree%\jscript\jshost -GenerateValidPointersMapHeader:%comparisonFile% dummy.js
if "%errorlevel%" NEQ "0" (
    Echo errorlevel is %errorlevel%
    Echo %0 : error: Error generating bytecode file. Ensure %comparisonFile% is writable. 1>&2
    goto Exit;
)
diff -dw %baseFileName% %comparisonFile%
if "%errorlevel%" NEQ "0" (
    Echo errorlevel is %errorlevel%
    Echo Engine modifications require that the ValidPointersMap be regenerated. 1>&2
    Echo Run %JSCRIPT_ROOT%\Lib\Common\Memory\ValidPointersMap\GenValidPointersMap.cmd 1>&2
    goto Exit;
)
Echo Pass

del %comparisonFile%

:Exit
exit /b 0

setlocal
call %*


cd %~dp0\..\..
echo %CD%
set JSCRIPT_ROOT=%CD%
call :GetName %CD%
set OBJECT_JSCRIPT_DIR=%OBJECT_ROOT%\inetcore\%_NAME%

echo JSCRIPT_ROOT=%JSCRIPT_ROOT%
echo OBJECT_JSCRIPT_DIR=%OBJECT_JSCRIPT_DIR%


set _SKIP_BYTE_CODE_VERIFY=1
build -cz -dir %JSCRIPT_ROOT%\core\manifests;%JSCRIPT_ROOT%\core\lib\backend;%JSCRIPT_ROOT%\core\lib\common;%JSCRIPT_ROOT%\core\lib\parser\release;%JSCRIPT_ROOT%\core\lib\runtime\bytecode\release;%JSCRIPT_ROOT%\core\lib\runtime\math;%JSCRIPT_ROOT%\core\lib\runtime\language;%JSCRIPT_ROOT%\core\lib\runtime\library;%JSCRIPT_ROOT%\core\lib\runtime\types;%JSCRIPT_ROOT%\private\lib\winrt;%JSCRIPT_ROOT%\private\lib\telemetry;%JSCRIPT_ROOT%\private\lib\engine;%JSCRIPT_ROOT%\private\lib\EnC;%JSCRIPT_ROOT%\private\lib\SCA;%JSCRIPT_ROOT%\private\lib\JsrtChakra;%JSCRIPT_ROOT%\private\lib\projection;%JSCRIPT_ROOT%\private\bin\chakra\test;%JSCRIPT_ROOT%\private\bin\jshost;%JSCRIPT_ROOT%\private\lib\memprotectheap\release;%JSCRIPT_ROOT%\private\lib\hostcommon;%JSCRIPT_ROOT%\private\lib\internalIDL;%JSCRIPT_ROOT%\private\lib\MemPRotectHeap;%JSCRIPT_ROOT%\core\lib\jsrt;%JSCRIPT_ROOT%\private\lib\scalookup

exit /B 0


:GetName
set _NAME=%~n1
exit /B0

setlocal
call %*


cd %~dp0\..\..\..\..
echo %CD%
set JSCRIPT_ROOT=%CD%
call :GetName %CD%
set OBJECT_JSCRIPT_DIR=%OBJECT_ROOT%\inetcore\%_NAME%

echo JSCRIPT_ROOT=%JSCRIPT_ROOT%
echo OBJECT_JSCRIPT_DIR=%OBJECT_JSCRIPT_DIR%


set _SKIP_BYTE_CODE_VERIFY=1
build -cz -dir %JSCRIPT_ROOT%\manifests;%JSCRIPT_ROOT%\lib\author;%JSCRIPT_ROOT%\lib\backend;%JSCRIPT_ROOT%\lib\common;%JSCRIPT_ROOT%\lib\parser\release;%JSCRIPT_ROOT%\lib\runtime\bytecode\release;%JSCRIPT_ROOT%\lib\runtime\math;%JSCRIPT_ROOT%\lib\runtime\language;%JSCRIPT_ROOT%\lib\runtime\library;%JSCRIPT_ROOT%\lib\runtime\types;%JSCRIPT_ROOT%\lib\winrt;%JSCRIPT_ROOT%\dll\jscript\test;%JSCRIPT_ROOT%\exe\common;%JSCRIPT_ROOT%\exe\jshost\release;%JSCRIPT_ROOT%\lib\memprotectheap\release

exit /B 0


:GetName
set _NAME=%~n1
exit /B0

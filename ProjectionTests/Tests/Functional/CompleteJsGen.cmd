@echo off
SETLOCAL ENABLEEXTENSIONS
:: This test exercise JsGen against a large set of winmd files.

SET _completeJsGenJSFile=%CD%\CompleteJsGen.js
SET _completeJsGenJSResponseFile=%CD%\CompleteJsGen.response

SET _jsHost=%WIN_JSHOST_METADATA_BASE_PATH%\JsHost.exe
SET _jsGen=%WIN_JSHOST_METADATA_BASE_PATH%\jsGen.exe
SET _baseFolder=%WIN_JSHOST_METADATA_BASE_PATH%

if not exist "%_jsHost%" (
    REM SNAP fall-back
	SET _jsHost=%_JSHostEXE%
)

if not exist "%_jsHost%" (
    echo  FAILED. FILE %_jsHost% DOESN'T EXIST
    exit /b 1
)

echo Removing prior CompleteJsGen.js file
del %_completeJsGenJSFile% 2>nul

del %_completeJsGenJSResponseFile% 2>nul

echo Writing CompleteJsGen.js
for /F %%i in ('dir /b %_baseFolder%\*.winmd') DO call :run_and_concat %%i 

echo JsGen CompleteJsGen.js
%_jsGen% %_completeJsGenJSFile% @%_completeJsGenJSResponseFile%

echo Checking for existence of CompleteJsGen.js
if not exist %_completeJsGenJSFile% (
    echo  FAILED. FILE DOESN'T EXIST
    exit /b 2
)

echo JsHost CompleteJsGen.js
%_jsHost% %_completeJsGenJSFile%

echo Done
goto :EOF
 
:run_and_concat 
echo %_baseFolder%\%1 >> %_completeJsGenJSResponseFile%
echo -r >> %_completeJsGenJSResponseFile%
echo %_baseFolder%\%1 >> %_completeJsGenJSResponseFile%

del %1.js 2>nul

echo  JsGen %1
%_jsGen% %_baseFolder%\%1 %1.js
echo /JsGen %1

if not exist %1.js (
   echo  FAILED For %_baseFolder%\%1
   exit /b 3
)

echo  JsHost %1
%_JsHost% %1.js
echo /JsHost %1

goto :eof 




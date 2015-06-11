@echo off
setlocal
set _FILE=
set _HASERROR=0
set _BUILD=1

:ParseArg
if "%1" == "" (
    goto :DoneParse
) else if /i "%1" == "nobuild" (
    set _BUILD=0
    goto :Parse1
) else if exist "%1"  (
    set _FILE=%_FILE% %1
    goto :Parse1
)

echo ERROR: Invalid parameter %1
exit /B 1

:Parse1
shift /1
goto :ParseArg

:DoneParse
set _RZARG=%_RazzleArguments%
set _RZARG=%_RZARG:amd64=%
set _RZARG=%_RZARG:x86=%
set _RZARG=%_RZARG:chk=%
set _RZARG=%_RZARG:fre=%
if "%_BUILD%" == "1" (
    echo Building x86chk
    start /WAIT /I %WINDIR%\SysWOW64\cmd.exe /C "%~dp0BuildGenByteCodeBin.cmd %RazzleToolPath%\razzle %_RZARG% x86chk" 
    if exist "%JSCRIPT_ROOT%\buildchk.err" (
        type %JSCRIPT_ROOT%\buildchk.err
    )
    echo Building amd64chk
    start /WAIT /I %WINDIR%\SysWOW64\cmd.exe /C "%~dp0BuildGenByteCodeBin.cmd %RazzleToolPath%\razzle %_RZARG% amd64chk" 
    if exist "%JSCRIPT_ROOT%\buildchk.err" (    
        type %JSCRIPT_ROOT%\buildchk.err
    )
)

if "%_FILE%" == "" (
    set "_FILE=%~dp0*.js"
)

for %%i in (%_FILE%) do (
    call :GenerateLibraryByteCodeHeader %%i
)
exit /B %_HASERROR%


:GenerateLibraryBytecodeHeader
set _BASE_PATH=%_NTTREE%
set _BASE_PATH=%_BASE_PATH:.x86chk=%
set _BASE_PATH=%_BASE_PATH:.amd64chk=%
set _BASE_PATH=%_BASE_PATH:.x86fre=%
set _BASE_PATH=%_BASE_PATH:.amd64fre=%
set _BASE_PATH=%_BASE_PATH:.armchk=%
set _BASE_PATH=%_BASE_PATH:.armfre=%

echo Generating %1.bc.32b.h
call :Generate %1 x86 %1.bc.32b.h
echo Generating %1.bc.64b.h
call :Generate %1 amd64 %1.bc.64b.h
exit /B 0

:Generate
sd edit %3 > nul
%_BASE_PATH%.%2chk\jscript\jshost -GenerateLibraryByteCodeHeader:%3 -Intl %1
if "%errorlevel%" NEQ "0" (
    echo %1: Error generating bytecode file. Ensure %3 writable.
    set _HASERROR=1
) else (
    echo Bytecode generated. Please rebuild to incorporate the new bytecode.
)

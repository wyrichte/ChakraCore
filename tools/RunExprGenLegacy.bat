@Echo Off
SETLOCAL ENABLEEXTENSIONS
Setlocal EnableDelayedExpansion
if "%1"=="-?" (
    echo Usage:
    echo    runexprgen.bat [threadCount] [copyFiles] [spanSeconds] [testcmdLine] [debuggerTestPercentage] [hybridDebuggerTestPercentage]
    echo        threadCount: Number of threads to use. Default: 4
    echo        copyFiles: [true or false] Determines if a copy of jshost.exe is made. Default: false
    echo        snapSeconds: Number of seconds to run exprgen for. Default:3600000
    echo        testcmdLine: Command line to be passed to exprgen
    echo        debuggerTestPercentage: Percentage of tests to run in debug mode. Default: 5% ^(25% of 5% in hybrid debugging mode^)
    echo        hybridDebuggerTestPercentage: Percentage of tests ^(out of debuggerTestPercentage^) to run in hybrid-debug mode. Default: 25% ^(25% of debuggerTestPercentage^)
    exit /b 0
)

ECHO Start Running Exprgen...
GOTO SETUP
:SETUP

set /a threadcount=%NUMBER_OF_PROCESSORS%
IF "%1" neq ""  (
  set /a threadcount=%1
)
set copyfiles=true
IF "%2" neq ""  (
  set copyfiles=%2
)
set /a spanseconds=3600000
IF "%3" neq ""  (
  set /a spanseconds=%3
)
set debuggerTestPercentage=0.05
set hybridDebuggerTestPercentage=0.25
IF "%5" neq ""  (
  set debuggerTestPercentage=%5
)
IF "%6" neq ""  (
  set hybridDebuggerTestPercentage=%6
)

set branch=%_BuildBranch%
CALL copyExprgenFiles.bat  %branch% %debuggerTestPercentage% %hybridDebuggerTestPercentage%
set MyVar=""
IF "%spanseconds%" == ""  (
  set /a spanseconds=3600000
)
IF NOT DEFINED threadcount (
  set /a threadcount=%NUMBER_OF_PROCESSORS%
)

GOTO DELETEFILES
GOTO EXIT

:DELETEFILES
ECHO Deleting JSHOST and Bug Files...
if exist jshost.exe del /Q JSHost.exe
if exist chakratest.dll del /Q chakratest.dll
del /Q *.js 2>NUL
del /Q New\bug*.js 2>NUL
del /Q Existing\bug*.js 2>NUL
IF "%copyfiles%" == "true" (
  GOTO COPYFILES
  )ELSE (
  GOTO RUNEXPRGEN
)
:COPYFILES
REM The first directory in the path is used for copying the jshost files
for /f "tokens=*" %%A in ( 'where jshost' ) do (
  if !MyVar! Equ "" (
    set  MyVar=%%~dpA
  )
)
set __Timestamp=%DATE%%TIME%
set TimeStamp=%__Timestamp:~10,4%%__Timestamp:~4,2%%__Timestamp:~7,2%_%__Timestamp:~14,2%%__Timestamp:~17,2%_%__Timestamp:~20,2%%__Timestamp:~23,2%
set TimeStamp=%TimeStamp: =0%

call :copyFile "ExprgenLog.txt" "ExprgenLog%TimeStamp%.txt"

REM if we need to copy more files in the future for Debugger we should add them here
call :copyFile "%MyVar%\jshost.exe" .\
call :copyFile "%MyVar%\chakratest.dll" .\
call :copyFile "%_NTTREE%\Symbols.pri\jscript\dll\chakratest.pdb" .\
call :copyFile "%_NTTREE%\Symbols.pri\jscript\exe\jshost.pdb" .\
call :copyFile "%MyVar%\jshost.pdb" .\
call :copyFile "%MyVar%\chakratest.pdb" .\

GOTO RUNEXPRGEN

:RUNEXPRGEN
IF [%4] EQU []   (
  Exprgen.exe -jshost -spanseconds:%spanseconds% -threads:%threadcount%
)Else (
  set testcmdline=%4
  Exprgen.exe -jshost -spanseconds:%spanseconds% -threads:%threadcount% -testcmdline:%testcmdline%
)
GOTO EXIT


:EXIT
goto :eof

:copyFile
setlocal ENABLEDELAYEDEXPANSION enableextensions
set file=%~1
set dst=%~2
if exist %file% (
   echo Copying %file% to %dst%
   COPY /Y %file% %dst%
) else (
   echo Skipping copy of %file% as it does not exist
)
endlocal & goto :eof

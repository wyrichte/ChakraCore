@Echo Off
SETLOCAL ENABLEEXTENSIONS
Setlocal EnableDelayedExpansion
if "%1"=="-?" (
    echo Usage:
    echo    runexprgen.bat [threadCount] [copyFiles] [spanSeconds] [testcmdline] [debuggerTestPercentage] [hybridDebuggerTestPercentage]
    echo        threadCount: Number of threads to use. Default: 4
    echo        copyFiles: [true or false] Determines if a copy of jshost.exe is made by the script. Otherwise Exprgen will make a copy. Default: false
    echo        snapSeconds: Number of seconds to run exprgen for. Default:3600000
    echo        testcmdline: Command line to be passed to exprgen
    echo        debuggerTestPercentage: Percentage of tests to run in debug mode. Default: 5% ^(25% of 5% in hybrid debugging mode^)
    echo        hybridDebuggerTestPercentage: Percentage of tests ^(out of debuggerTestPercentage^) to run in hybrid-debug mode. Default: 25% ^(25% of debuggerTestPercentage^)
    exit /b 0
)

ECHO Start Running Exprgen...
GOTO SETUP
:SETUP
if "%REPO_ROOT%" == "" (
  echo Please run root\tools\GitScripts\init.cmd first
  goto:eof
)

set /a threadcount=%NUMBER_OF_PROCESSORS%
set copyfiles=false
set testcmdline=
set /a spanseconds=3600000
set debuggerTestPercentage=0.05
set hybridDebuggerTestPercentage=0.25

IF "%1" neq ""  (
  set /a threadcount=%1
)
IF "%2" neq ""  (
  set copyfiles=%2
)
IF "%3" neq ""  (
  set /a spanseconds=%3
)
IF "%4" neq ""  (
  set testcmdline=-testcmdline:%4
)
IF "%5" neq ""  (
  set debuggerTestPercentage=%5
)
IF "%6" neq ""  (
  set hybridDebuggerTestPercentage=%6
)

set _ChakraBuildConfig=
if "%_BuildType%" EQU "chk" (
    set _ChakraBuildConfig=Debug
) else if "%_BuildType%" EQU "fre" (
    set _ChakraBuildConfig=Test
) else if "%_BuildType%" EQU "test" (
    set _ChakraBuildConfig=Test
) else (
    echo WARNING: Unknown build type '%_BuildType%'
)
set flavor=%_BuildArch%_%_ChakraBuildConfig%
set ExprgenDestination=%REPO_ROOT%\tools\Exprgen\%flavor%
CALL %~dp0\copyExprgenFiles.bat "%ExprgenDestination%" %debuggerTestPercentage% %hybridDebuggerTestPercentage%

set ExprGenRunPath=%ExprgenDestination%\Runs
md %ExprGenRunPath%
pushd %ExprGenRunPath%

set Binaries=jshost.exe jshost.pdb chakra.dll chakra.pdb jdtest.exe jdtest.pdb
set ChakraBinPath=%REPO_ROOT%\Build\VcBuild\bin\%flavor%

:DELETEFILES
ECHO Deleting JSHOST and Bug Files...
del /Q *.js 2>NUL
del /Q New\bug*.js 2>NUL
del /Q Existing\bug*.js 2>NUL
IF "%copyfiles%" == "true" (
  GOTO COPYFILES
) ELSE (
  GOTO RUNEXPRGEN
)

:COPYFILES
set __Timestamp=%DATE%%TIME%
set TimeStamp=%__Timestamp:~10,4%%__Timestamp:~4,2%%__Timestamp:~7,2%_%__Timestamp:~14,2%%__Timestamp:~17,2%_%__Timestamp:~20,2%%__Timestamp:~23,2%
set TimeStamp=%TimeStamp: =0%

call :copyFile "ExprgenLog.txt" "ExprgenLog%TimeStamp%.txt"

REM if we need to copy more files in the future for Debugger we should add them here
robocopy %ChakraBinPath% %ExprGenRunPath%\ %Binaries%

GOTO RUNEXPRGEN

:RUNEXPRGEN
set ExprgenCopyBinaries=-copybinaries
IF "%copyfiles%" == "true" (
  REM If the script copied the binaries, tell exprgen not to do it
  set ExprgenCopyBinaries=
)

ECHO %ExprgenDestination%\Exprgen.exe -jshost -spanseconds:%spanseconds% -threads:%threadcount% %testcmdline% -testbinarysourcepath:%ChakraBinPath% %ExprgenCopyBinaries% -snippets:%ExprgenDestination%
start %ExprgenDestination%\Exprgen.exe -jshost -spanseconds:%spanseconds% -threads:%threadcount% %testcmdline% -testbinarysourcepath:%ChakraBinPath% %ExprgenCopyBinaries% -snippets:%ExprgenDestination%
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

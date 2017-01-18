:: Regenerate all bytecode in FULL and CORE.
:: jshost.exe is used to generate Promise bytecodes in FULL.
:: ch.exe is used to generate Intl bytecodes in CORE.
:: ch.exe (NoJIT variety) is used to generate NoJIT Intl bytecodes in CORE.
:: Each set of bytecode requires an x86_debug and x64_debug binary.
::
:: Thus we need to build the following:
:: [Full]   jshost.exe    x64_debug
:: [Full]   jshost.exe    x86_debug
:: [Core]   ch.exe        x64_debug
:: [Core]   ch.exe        x86_debug
:: [Core]   ch.exe        x64_debug (NoJIT)
:: [Core]   ch.exe        x86_debug (NoJIT)
::
:: NOTE: It may be possible to build regular Intl bytecodes from jshost.exe and thus only build NoJIT ch.exe
:: NOTE: but we decided against this for now because of minor differences in the output.

@echo off
setlocal

set DoFull=1
set DoCore=1
set __ScriptDirectory=%~dp0

:ContinueArgParse
if not "%1"=="" (
  if "%1"=="--no-full" (
    set DoFull=0
  )
  if "%1"=="--no-core" (
    set DoCore=0
  )
  if "%1"=="/?" (
    goto:showHelp
  )
  if "%1"=="-h" (
    goto:showHelp
  )
  if "%1"=="--help" (
    goto:showHelp
  )
  shift
  goto :ContinueArgParse
)

pushd %__ScriptDirectory%..

if %DoFull%==1 (
  :: [Full]   jshost.exe    x64_debug
  call tools\GitScripts\init.cmd x64 debug
  call build /verbosity:minimal
  if %errorlevel% neq 0 (
    echo There was a build error for x64 debug. Stopping bytecode generation.
    exit /b 1
  )
  :: [Full]   jshost.exe    x86_debug
  call tools\GitScripts\init.cmd x86 debug
  call build /verbosity:minimal
  if %errorlevel% neq 0 (
    echo There was a build error for x64 debug. Stopping bytecode generation.
    exit /b 1
  )

  :: Generate Promise Bytecodes
  pushd private\lib\Projection\InJavascript
  call GenByteCode.cmd
  if %errorlevel% neq 0 (
    echo There was an error when regenerating bytecode headers.
    exit /b 1
  )
  popd
)


if %DoCore%==1 (
  call core/RegenAllByteCode.cmd
)

popd

endlocal
goto:eof

:showHelp
echo usage RegenAllByteCode.cmd
echo --no-full   Will not regenerate bytecode header for ChakraFull
echo --no-core   Will not regenerate bytecode header for ChakraCore
echo -h, --help, /? Show this help message
endlocal

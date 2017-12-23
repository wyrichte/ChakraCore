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
  call :RegenFull
)

if %DoCore%==1 (
  call core/RegenAllByteCode.cmd
)

popd

endlocal
goto:eof

:RegenFull
  :: [Full]   jshost.exe    x64_debug
  call tools\GitScripts\init.cmd x64 debug
  call build /verbosity:minimal
  if %errorlevel% neq 0 (
    echo There was a build error for x64 debug. Stopping bytecode generation.
    exit /b 1
  )

  :: Verify ByteCode GUID
  set __bin=%__ScriptDirectory%..\Build\VcBuild\bin\x64_debug\jshost.exe
  set __baseline=%__ScriptDirectory%..\unittest\Miscellaneous\ByteCodeVerification.baseline
  set __veriflog=%__ScriptDirectory%verification.log
  call %__bin% -ByteCodeVerification %__baseline% > %__veriflog%
  if %errorlevel% equ 3 (
    echo GUID Mismatch, the guid was probably changed and require a baseline update
    call %__bin% -ByteCodeVerification > %__baseline%
  )
  if %errorlevel% equ 4 (
    echo OpCodes changed, require a GUID update, the new GUID should be in the log file
    copy /y %__veriflog% %__ScriptDirectory%..\core\lib\Runtime\ByteCode\ByteCodeCacheReleaseFileVersion.h
    call:convert2Unix %__ScriptDirectory%..\core\lib\Runtime\ByteCode\ByteCodeCacheReleaseFileVersion.h
    :: Need to rebuild to validate update to GUID
    goto:RegenFull
  )
  :: Run a second time, there should be no errors now
  call %__bin% -ByteCodeVerification %__baseline% > %__veriflog%
  if %errorlevel% neq 0 (
    echo There was an error while running ByteCodeVerification
    type %__veriflog%
    del %__veriflog%
    exit /b 1
  )
  del %__veriflog%

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
goto:eof

:showHelp
  echo usage RegenAllByteCode.cmd
  echo --no-full   Will not regenerate bytecode header for ChakraFull
  echo --no-core   Will not regenerate bytecode header for ChakraCore
  echo -h, --help, /? Show this help message
  endlocal
goto:eof

:convert2Unix
  where dos2unix.exe > NUL
  if %errorlevel% equ 0 (
    dos2unix.exe %1
    goto:eof
  )
  echo todo: Find a way to fix line endings without dos2unix utility
goto:eof

@echo off
REM Usage: WarnGenByteCode <jsfile> <tempfile> <headerfile>
IF "%_SKIP_BYTE_CODE_VERIFY%" == "1" (
    echo %1: Library byte code Verification skipped 1>&2
    exit /B 0
)
setlocal
echo // Copyright (C) Microsoft. All rights reserved.>%2
echo // Generated via jshost -GenerateLibraryByteCodeHeader>>%2
echo #if 0 >>%2
type %1 >> %2
echo #endif>> %2
fc /A %2 %3 | findstr /n /C:"namespace Js" > %2.diff
echo 5:namespace Js > %2.expected
fc %2.diff %2.expected > nul
IF NOT "%ERRORLEVEL%" == "0" (
    echo WarnGenByteCode : error: %1 has been modified and %3 must be regenerated. Build with _SKIP_BYTE_CODE_VERIFY=1 and run InJavaScript\GenByteCode.cmd %1. 1>&2
    exit /b -1
)
exit /B 0

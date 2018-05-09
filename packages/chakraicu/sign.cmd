@echo off
setlocal

for %%a in (x64 x86 ARM ARM64) do (
    set "NTSIGNARCH=%%a"
    if "%%a"=="x64" (
        set NTSIGNARCH=amd64
    )

    ntsign -arch:%NTSIGNARCH% "%1\%%a\Chakra.ICU.Common.dll" > nul
    signtool verify "%1\%%a\Chakra.ICU.Common.dll"
    ntsign -arch:%NTSIGNARCH% "%1\%%a\icuuc.dll" > nul
    signtool verify "%1\%%a\icuuc.dll"
    ntsign -arch:%NTSIGNARCH% "%1\%%a\icuin.dll" > nul
    signtool verify "%1\%%a\icuin.dll"
    ntsign -arch:%NTSIGNARCH% "%1\%%a\Chakra.ICU.Data.dll" > nul
    signtool verify "%1\%%a\Chakra.ICU.Data.dll"
)

endlocal

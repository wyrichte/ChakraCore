@echo off
setlocal

if "%1"=="" (
    echo Usage: PerfRun.cmd tool.exe
    goto END
)
set APP=%1
set TESTS=(3d-cube 3d-morph 3d-raytrace access-binary-trees access-fannkuch access-nbody access-nsieve bitops-3bit-bits-in-byte bitops-bits-in-byte bitops-bitwise-and bitops-nsieve-bits controlflow-recursive math-cordic math-partial-sums math-spectral-norm regexp-dna string-base64 string-fasta)

for %%a in %TESTS% do (
    echo Testing %%a
    %APP% %2 %%a.js
    %APP% %2 %%a.js
    %APP% %2 %%a.js
)

:END
endlocal

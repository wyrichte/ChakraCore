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

setlocal

:: [Full]   jshost.exe    x64_debug
call tools\GitScripts\init.cmd x64 debug
call build /verbosity:minimal
:: [Full]   jshost.exe    x86_debug
call tools\GitScripts\init.cmd x86 debug
call build /verbosity:minimal

:: Generate Promise Bytecodes
pushd private\lib\Projection\InJavascript
call GenByteCode.cmd
popd

:: NOTE: For now, we will not generate Intl Bytecodes using jshost.exe because of minor differences in the output.
:: NOTE: If we decide to generate Intl Bytecodes with jshost.exe later, uncomment this block.
REM :: Generate Intl Bytecodes using jshost.exe
REM pushd core\lib\Runtime\Library\InJavascript
REM call GenByteCode.cmd -binary jshost.exe -bindir ..\..\..\..\..\Build\VcBuild\bin
REM popd

pushd core

:: NOTE: If we decide to generate Intl Bytecodes with jshost.exe later, comment this block.
:: [Core]   ch.exe        x64_debug
:: [Core]   ch.exe        x86_debug
call jenkins\buildone.cmd x64 debug
call jenkins\buildone.cmd x86 debug
pushd lib\Runtime\Library\InJavascript
call GenByteCode.cmd
popd

:: [Core]   ch.exe        x64_debug (NoJIT)
:: [Core]   ch.exe        x86_debug (NoJIT)
call jenkins\buildone.cmd x64 debug "/p:BuildJIT=false"
call jenkins\buildone.cmd x86 debug "/p:BuildJIT=false"
:: Generate Intl NoJIT Bytecodes using ch.exe (NoJIT)
pushd lib\Runtime\Library\InJavascript
call GenByteCode.cmd -nojit
popd

popd

endlocal

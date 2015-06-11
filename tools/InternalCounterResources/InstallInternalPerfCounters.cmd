@echo off
IF "%1" == "" (
    echo "Usage: InstallInternalPerfCounters <path to JScript9InternalCounters.dll>
    exit /B 1
)

IF NOT EXIST "%1\Jscript9InternalCounters.dll" (
    echo %1\Jscript9InternalCounters.dll does not exist
    exit /B 1
)

rem install counters
unlodctr /m:%~dp0\..\..\Manifests\Microsoft-Scripting-JScript9.InternalCounters.man 
lodctr /m:%~dp0\..\..\Manifests\Microsoft-Scripting-JScript9.InternalCounters.man %1


rem install ETW events
wevtutil um %~dp0\..\..\Manifests\Microsoft-Scripting-JScript9.Internal.man
wevtutil im /rf:%1\Jscript9InternalCounters.dll /mf:%1\Jscript9InternalCounters.dll %~dp0\..\..\Manifests\Microsoft-Scripting-JScript9.Internal.man


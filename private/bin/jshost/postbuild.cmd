:: ============================================================================
::
:: postbuild.cmd
::
:: Performs post-build setup for jshost so that jshost has all necessary
:: binaries available for running tests such as the debugger tests.
::
:: ============================================================================
@echo off
setlocal

set _BuildArch=%1
set _OutputDir=%~f2
set _IELibsDir=%~f3
set _RemoteDir=%~f4

:: Copy PDM files to jshost.exe.local in build output directory
set _PdmFiles=%_IELibsDir%\pdm\%_BuildArch%
set _TargetDir=%_OutputDir%jshost.exe.local

mkdir "%_TargetDir%"

echo Robocopy *.dll from "%_PdmFiles%" to "%_TargetDir%"
%WinDir%\System32\robocopy /NS /NC /NFL /NDL /NP /NJS /NJH /E *.dll "%_PdmFiles%" "%_TargetDir%"

if not [%_RemoteDir%]==[] (
    if "%_BuildArch%" EQU "ARM64" (
        echo Robocopy from "%_OutputDir%" to "%_RemoteDir%"
        %WinDir%\System32\robocopy /NS /NC /NFL /NDL /NP /NJS /NJH /E * %_OutputDir% %_RemoteDir%
    )
)

exit /b 0

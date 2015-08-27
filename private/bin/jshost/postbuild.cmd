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

:: Copy PDM files to jshost.exe.local in build output directory
set _PdmFiles=%_IELibsDir%\pdm\%_BuildArch%\*.dll
set _TargetDir=%_OutputDir%jshost.exe.local

mkdir "%_TargetDir%"

echo Copying "%_PdmFiles%" to "%_TargetDir%"
echo.
copy /Y "%_PdmFiles%" "%_TargetDir%"

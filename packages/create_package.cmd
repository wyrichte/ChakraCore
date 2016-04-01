:: Creates a package for the given directory
:: Script parameters are controlled through environment variables
:: 
:: NUGET_PACKAGE_OUT: Directory in which the generated package is stored
:: NUGET_PACKAGE_SOURCE_DIR: Directory which contains the NuSpec file that describes the package
:: NUGET_PATH: Path to Nuget.exe 
:: NUGET_BASE_PATH: The directory that all the files listed in the NuSpec file is relative to
:: NUGET_VERBOSITY: Parameters controlling how verbose the NuGet output is
::

@echo off

setlocal
setlocal EnableDelayedExpansion

if "%NUGET_PACKAGE_OUT%" EQU "" (
    set NUGET_PACKAGE_OUT=%SDXROOT%\onecoreuap\inetcore\jscript\packages\package_output
    echo Setting package output directory to %NUGET_PACKAGE_OUT%
)

if "%NUGET_PACKAGE_SOURCE_DIR%" EQU "" (
    set NUGET_PACKAGE_SOURCE_DIR=.
)

if not exist %NUGET_PACKAGE_OUT% (
    mkdir %NUGET_PACKAGE_OUT%
)

if "%NUGET_PATH%" EQU "" (
    set NUGET_PATH=\\chakrafs01\Nuget\support
)

if "%NUGET_BASE_PATH%" EQU "" (
    echo ERROR: NUGET_BASE_PATH env variable is not defined. Are you running under Razzle?
    goto :Exit
)

:: TODO: use the following to pass the version explictly to nuget. This way we don't need to update the versions in the nuspec files.
:: perl -nle "print $1 if /%NUGET_PACKAGE_SOURCE_DIR%.*(\d+\.\d+\.\d+)/" %SDXROOT%\onecoreuap\inetcore\jscript\packages.config

:: For more diagnostics, set the following variable to 'normal' or 'detailed'
set NUGET_VERBOSITY=-Verbosity quiet 
%NUGET_PATH%\nuget pack %NUGET_PACKAGE_SOURCE_DIR%\package.nuspec %NUGET_VERBOSITY% -BasePath %NUGET_BASE_PATH% -OutputDirectory %NUGET_PACKAGE_OUT%

:Exit
endlocal

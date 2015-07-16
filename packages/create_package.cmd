:: Creates a package for the given directory
:: Script parameters are controlled through environment variables
:: 
:: NUGET_PACKAGE_OUT: Directory in which the generated package is stored
:: NUGET_PACKAGE_SOURCE_DIR: Directory which contains the NuSpec file that describes the package
:: NUGET_PATH: Path to Nuget.exe 
:: NUGET_BASE_PATH: The directory that all the files listed in the NuSpec file is relative to
:: NUGET_VERBOSITY: Parameters controlling how verbose the NuGet output is
::

:: @echo off

setlocal

if "%NUGET_PACKAGE_OUT%" EQU "" (
    set NUGET_PACKAGE_OUT=%SDXROOT%\inetcore\jscript\packages\package_output
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

:: For more diagnostics, run set the following variable
:: Set NUGET_VERBOSITY=-Verbosity detailed 
%NUGET_PATH%\nuget pack %NUGET_PACKAGE_SOURCE_DIR%\package.nuspec %NUGET_VERBOSITY% -BasePath %NUGET_BASE_PATH% -OutputDirectory %NUGET_PACKAGE_OUT%

:Exit
endlocal

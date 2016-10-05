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
    echo Setting package output directory to !%NUGET_PACKAGE_OUT!
)

if "%NUGET_PACKAGE_SOURCE_DIR%" EQU "" (
    set NUGET_PACKAGE_SOURCE_DIR=.
)

if not exist %NUGET_PACKAGE_OUT% (
    mkdir %NUGET_PACKAGE_OUT%
)

if "%NUGET_PATH%" EQU "" (
    set NUGET_PATH=\\chakrafs\fs\Misc\NuGet\Support
)

if "%NUGET_BASE_PATH%" EQU "" (
    echo ERROR: NUGET_BASE_PATH env variable is not defined. Are you running under Razzle?
    goto :Exit
)

:: Use the following to pass the version explictly to nuget. This way we don't need to update the versions in the nuspec files.
for /f tokens^=4^ delims^=^" %%i in ('findstr /R "%NUGET_PACKAGE_NAME%.*[0-9]\.[0-9]\.[0-9]" %SDXROOT%\onecoreuap\inetcore\jscript\packages.config') do set NUGET_PACKAGE_VERSION=%%i

if [%NUGET_PACKAGE_VERSION%]==[] (
  echo %NUGET_PACKAGE_NAME% package version not found in packages.config!
  goto :Exit
)

echo Packaging %NUGET_PACKAGE_NAME% %NUGET_PACKAGE_VERSION%...

:: For more diagnostics, set the following variable to 'normal' or 'detailed'
set NUGET_VERBOSITY=-Verbosity normal
%NUGET_PATH%\nuget pack %NUGET_PACKAGE_SOURCE_DIR%\package.nuspec %NUGET_VERBOSITY% -NoPackageAnalysis -Version %NUGET_PACKAGE_VERSION% -BasePath %NUGET_BASE_PATH% -OutputDirectory %NUGET_PACKAGE_OUT%

:Exit
endlocal

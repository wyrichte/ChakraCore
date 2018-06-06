@REM Wrapper to run RunJSDRT.ps1 from PublishDrt script

@echo off
set BUNDLEROOT=%1
set LOGSDIR=%BUNDLEROOT%\LogFiles
set FILENAME=%~f0
set ARCH=%architecture%
@REM The BUNDLEROOT needs to have flavor as the last 3 characters for this to work correctly e.g. c:\CopyBuiltFiles.x86.chk
set FLAVOR=%BUNDLEROOT:~-3%
@REM if %FILENAME:fre=%==%FILENAME% set FLAVOR=chk

echo BundleRoot is at %BUNDLEROOT%
echo Logs are at %LOGSDIR%
echo Architecture is %ARCH%
echo Flavor is %FLAVOR%
echo FILENAME is %FILENAME%

echo powershell.exe -ExecutionPolicy Bypass %BUNDLEROOT%\RunJSDRT.ps1 -RunUnitTests $True -RunHtmlUnitTests $False -RunJsrtUnitTests $False -DrtBundleRoot %BUNDLEROOT% -DrtMode $True -BuildArch %ARCH% -BuildFlavor %FLAVOR% -LogsRootDir %LOGSDIR% -Dirs crossthread`,jd`,Math`,Function`,Intl`,IntlCore`,InternalProfile`,Error`,Array`,Bugs`,typedarray`,FixedFields`,AsyncDebug
powershell.exe -ExecutionPolicy Bypass %BUNDLEROOT%\RunJSDRT.ps1 -RunUnitTests $True -RunHtmlUnitTests $False -RunJsrtUnitTests $False -DrtBundleRoot %BUNDLEROOT% -DrtMode $True -BuildArch %ARCH% -BuildFlavor %FLAVOR% -LogsRootDir %LOGSDIR% -Dirs crossthread`,jd`,Math`,Function`,Intl`,IntlCore`,InternalProfile`,Error`,Array`,Bugs`,typedarray`,FixedFields`,AsyncDebug

if ERRORLEVEL 1 (
    echo [LABMARKFAILED] >> %LOGSDIR%\Test-Summary.log
    echo Execution had non-zero exitcode. See %LOGSDIR% folder for detailed logs.
) else (
    echo [LABMARKPASSED] >> %LOGSDIR%\Test-Summary.log
    echo Execution passed.
)

exit /b %ERRORLEVEL%

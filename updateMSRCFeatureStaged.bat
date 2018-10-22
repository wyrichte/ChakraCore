rem This is to update public for the following file
rem %_NTBINDIR%\onecore\base\wil\staging\FeatureStaging-MSRC-GlobalSettings.xml

setlocal
pushd .
cd /d %_NTBINDIR%\onecore\base\wil\staging

call Validate-MSRC-featureStaging-XML.cmd

@echo off
if %ERRORLEVEL% == 0 (
    echo build -c -z
    build -c -z
) else (
    echo ==== NOTE: Stopped and skipped build due to error in XML validation [exit %ERRORLEVEL%] ====
)
@echo on

popd
endlocal

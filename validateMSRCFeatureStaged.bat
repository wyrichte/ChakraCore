rem This is to validate the following file
rem %_NTBINDIR%\onecore\base\wil\staging\FeatureStaging-MSRC-GlobalSettings.xml

setlocal
pushd .
cd /d %_NTBINDIR%\onecore\base\wil\staging

call Validate-MSRC-featureStaging-XML.cmd

popd
endlocal

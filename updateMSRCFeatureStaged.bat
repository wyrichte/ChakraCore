pushd .
rem This is to update public for the following file
rem %_NTBINDIR%\onecore\base\wil\staging\FeatureStaging-MSRC-GlobalSettings.xml
cd /d %_NTBINDIR%\onecore\base\wil\staging
build -c -z
popd
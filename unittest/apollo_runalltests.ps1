cdd C:\data\test\browser_inetcore_test_package
mdd logs 
mdd logs\default 
mdd logs\dynapogo 
mdd logs\interpreted 
execd -output ..\bin\runalltests.cmd "-arch $env:_BuildArch -type $env:_BuildType"

$env:JSUTSummaryPattern="^Summary.*failures$"
echo "">_summary.txt
echo "">>_summary.txt
echo "<---- Unit Test Summary ---->" >> _summary.txt
echo "">>_summary.txt

cdd logs\interpreted
getd rl.log.* >x
del x
echo "">>_summary.txt
echo "">>_summary.txt
echo "<---- Interpreted ---->" >> _summary.txt
findstr /R $env:JSUTSummaryPattern rl.log >> _summary.txt
cdd ..\..

cdd logs\dynapogo
getd rl.log.* >x
del x
echo "">>_summary.txt
echo "">>_summary.txt
echo "<---- Dynapogo ---->" >> _summary.txt
findstr /R $env:JSUTSummaryPattern rl.log >> _summary.txt
cdd ..\..

cdd logs\default
getd rl.log.* >x
del x
echo "">>_summary.txt
echo "">>_summary.txt
echo "<---- Default ---->" >> _summary.txt
findstr /R $env:JSUTSummaryPattern rl.log >> _summary.txt
type _summary.txt
del _summary.txt
del rl.log
cdd ..\..


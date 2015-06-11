//Configuration: stackwalking.xml
//Testcase Number: 9082
//Bailout Testing: ON
//Switches: -version:2  -maxinterpretcount:1 -maxsimplejitruncount:1  -MinSwitchJumpTableSize:2 -off:polymorphicinlinecache -bgjit- -loopinterpretcount:1 -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -force:fieldcopyprop -forceserialized -force:fieldhoist -sse:0 -force:inline -force:atom -off:lossyinttypespec -off:aggressiveinttypespec -off:trackintusage -force:fixdataprops -force:rejit -off:ArrayCheckHoist
//Baseline Switches: -nonative  -version:2
//Arch: X86
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 140730-2030
//FullBuild: 9802.0.140730
//MachineName: VSP75819
//InstructionSet: 
//reduced switches: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -force:inline
var __counter = 0;
function test0() {
  __counter++;
  function leaf() {
  }
  var obj0 = {};
  var obj1 = {};
  var arrObj0 = {};
  var func0 = function () {
    for (var _strvar0 in obj0) {
      if (_strvar0.indexOf('method') != -1) {
        continue;
      }
      return leaf();
    }
    return leaf();
    do {
    } while (arrObj0);
  };
  var func2 = function () {
  };
  obj0.method0 = func2;
  obj0.method1 = func0;
  arrObj0.method1 = func2;
  Object.prototype.prop0 = -21449704;
  var uniqobj27 = [
      obj1,
      obj0,
      arrObj0
    ];
  var uniqobj28 = uniqobj27[__counter];
  uniqobj28.method1();
}
try
{
	test0();
	test0();
	test0();
}
catch(e)
{
	WScript.Echo("PASS");
}
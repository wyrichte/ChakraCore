//Configuration: lessmath_variableMRU.xml
//Testcase Number: 19319
//Switches: -maxinterpretcount:1 -maxsimplejitruncount:1  -MinSwitchJumpTableSize:3 -forcedeferparse -force:fieldhoist -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -bgjit- -loopinterpretcount:1 -force:fieldcopyprop -force:polymorphicinlinecache -force:interpreterautoprofile -force:inline -force:atom -force:rejit -off:lossyinttypespec -off:BoundCheckElimination -off:ArrayCheckHoist -off:ArrayLengthHoist -off:aggressiveinttypespec -force:fixdataprops -force:ScriptFunctionWithInlineCache -off:trackintusage -off:fefixedmethods -ForceArrayBTree -off:BoundCheckHoist -off:checkthis
//Baseline Switches: -nonative
//Arch: AMD64
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 140814-2030
//FullBuild: 9813.0.140814
//MachineName: BPT04023
//InstructionSet: 
//reduced switches: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -force:inline
function test0() {
  var GiantPrintArray = [];
  var v26 = {};
  Object.defineProperty(Object.prototype, '__getterprop4', {
    get: function () {
      function v0() {
      }
      v0.prototype.v2 = function () {
      };
      var v3 = new v0();
      function v4() {
      }
      v4.prototype.v2 = function () {
      };
      var v6 = new v4();
      function v17(v18) {
        v18.v2();
      }
      v17(v3);
      v17(v6);
    }, configurable:true
  });
  GiantPrintArray.push(v26.__getterprop4);
  for (;;) {
    break;
	
    for (var _strvar0 in IntArr0) {
    }
    GiantPrintArray.push(v30.__getterprop4);
  }
}
test0();
test0();
test0();
WScript.Echo("PASS");

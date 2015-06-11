//Configuration: cse.xml
//Testcase Number: 8714
//Switches: -PrintSystemException   -maxinterpretcount:1 -maxsimplejitruncount:2  -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -off:fefixedmethods -force:atom -force:fixdataprops -force:rejit -ForceArrayBTree -force:ScriptFunctionWithInlineCache -off:lossyinttypespec -off:checkthis -off:ParallelParse -off:ArrayCheckHoist -off:DelayCapture -off:aggressiveinttypespec -off:bailonnoprofile -RecyclerConcurrentStress -RecyclerVerifyMark -RecyclerConcurrentStress -off:NativeArray -off:ArrayLengthHoist -off:BoundCheckHoist -off:trackintusage -ValidateInlineStack -ForceCleanPropertyOnCollect -ForceDecommitOnCollect -MaxTrackedObjectListCount:1 -PageHeap:2 -off:BoundCheckElimination -off:objtypespec  -MaxMarkStackPageCount:1 -off:LoopCountBasedBoundCheckHoist -off:ArrayMissingValueCheckHoist
//Baseline Switches: -nonative  -PrintSystemException
//Arch: AMD64
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 140919-2030
//FullBuild: 9836.0.140919
//MachineName: BPT42363
//InstructionSet: 
//reduced switches: -maxsimplejitruncount:2 -maxinterpretcount:1
//noRepro switches1: -maxsimplejitruncount:2 -maxinterpretcount:1 -off:DynamicProfile
function test0() {
  var obj0 = {};
  var obj1 = {};
  var func0 = function () {
    function foo() {
      eval('');
    }
  };
  var protoObj0 = Object(obj0);
  obj1.prop0 = -1491889879;
  Object.defineProperty(protoObj0, 'prop0', {
    set: function () {
    }
  });
  WScript.Echo(protoObj0.prop0);
  WScript.Echo(obj1.prop0);
}
test0();
Debug.setAutoProxyName('test0');
test0();

// === Baseline Output ===
// command: D:\CAS\Debug\JsReducer\JSHost.exe -nonative step534.js
// exitcode: 0
// stdout:
// undefined
// -1491889879
// undefined
// undefined
// 
// stderr:
// 
// 
// 
// === Actual Output ===
// command: D:\CAS\Debug\JsReducer\JSHost.exe -maxsimplejitruncount:2 -maxinterpretcount:1 step534.js
// exitcode: 0
// stdout:
// undefined
// -1491889879
// undefined
// -1491889879
// 
// stderr:
// 
// 
// 

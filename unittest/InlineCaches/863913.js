//Configuration: constructors.xml
//Testcase Number: 10544
//Switches: -PrintSystemException   -maxinterpretcount:9 -maxsimplejitruncount:6  -force:fieldcopyprop -MinSwitchJumpTableSize:3 -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -bgjit- -loopinterpretcount:1 -forcedeferparse -force:fieldhoist -off:simplejit -force:polymorphicinlinecache -sse:0 -off:checkthis -force:rejit -force:atom -off:aggressiveinttypespec -force:fixdataprops -off:DelayCapture -force:ScriptFunctionWithInlineCache -off:lossyinttypespec -ForceArrayBTree -off:ArrayCheckHoist -off:bailonnoprofile -off:ParallelParse -off:trackintusage -ValidateInlineStack -RecyclerConcurrentStress -RecyclerVerifyMark -off:fefixedmethods -off:BoundCheckHoist -off:JsArraySegmentHoist -PageHeap:2 -off:ArrayLengthHoist -off:LdLenIntSpec -ForceCleanPropertyOnCollect -ForceDecommitOnCollect -off:LoopCountBasedBoundCheckHoist -MaxTrackedObjectListCount:1 -off:BoundCheckElimination -RecyclerConcurrentStress -stress:BailOnNoProfile  -SkipFuncCountForBailOnNoProfile:5  -MaxMarkStackPageCount:1 -off:NativeArray -off:objtypespec  -off:EliminateArrayAccessHelperCall -off:ArrayMissingValueCheckHoist
//Baseline Switches: -nonative  -PrintSystemException
//Arch: AMD64
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 140920-2030
//FullBuild: 9836.0.140920
//MachineName: BPT41510
//InstructionSet: 
//reduced switches: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:6 -maxinterpretcount:9 -pageheap:2
function test0() {
}
 Debug.setAutoProxyName(test0);
var protoObj1 = {};
Object.prototype.prop0 = 2147483650;
function func8() {
}
obj8 = new func8();
Object.defineProperty(protoObj1, 'prop0', {
  set: function () {
  }
});
WScript.Echo(protoObj1.prop0);
WScript.Echo(Object.prop0);
WScript.Echo(obj8.prop0);

// === Baseline Output ===
// command: D:\CAS-Exprgen-AMD64-2\JsReducer\JSHost.exe -nonative step551.js
// exitcode: 0
// stdout:
// undefined
// 2147483650
// undefined
// 
// stderr:
// 
// 
// 
// === Actual Output ===
// command: D:\CAS-Exprgen-AMD64-2\JsReducer\JSHost.exe -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:6 -maxinterpretcount:9 -pageheap:2 step551.js
// exitcode: 0
// stdout:
// undefined
// 2147483650
// 2147483650
// 
// stderr:
// 
// 
// 

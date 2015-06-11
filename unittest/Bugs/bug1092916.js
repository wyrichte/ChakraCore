//Configuration: TrackIntUsage.xml
//Testcase Number: 9868
//Bailout Testing: ON
//Switches: -PrintSystemException   -maxinterpretcount:1 -maxsimplejitruncount:2 -werexceptionsupport  -off:polymorphicinlinecache -MinSwitchJumpTableSize:2 -bgjit- -loopinterpretcount:1 -forcedeferparse -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -force:fieldhoist -force:ScriptFunctionWithInlineCache -force:rejit -off:lossyinttypespec -off:ArrayCheckHoist -force:atom -force:fixdataprops -off:ParallelParse -off:checkthis -ValidateInlineStack -off:DelayCapture -ForceArrayBTree
//Baseline Switches: -nonative -werexceptionsupport  -PrintSystemException
//Arch: X86
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 141112-2030
//FullBuild: 9879.0.141112
//MachineName: BPT11310
//InstructionSet: 
//reduced switches: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:1
//noRepro switches1: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:1 -off:InterpreterProfile
//noRepro switches2: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:1 -off:DynamicProfile
//noRepro switches3: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:1 -off:MarkTempObject
//noRepro switches4: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:1 -off:MarkTemp
//noRepro switches5: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:1 -off:FieldCopyProp
function test0() {
  var func3 = function (argMath3) {
    argMath3 = {};
    argMath3.prop0 = argMath3;
    argMath3.prop0;
  };
  func3();
}
test0();
test0();
test0();
test0();
WScript.Echo("PASSED");
// === Output ===
// command: D:\CAS-Exprgen-X86-3\JsReducer\JSHost.exe -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:2 -maxinterpretcount:1 step336.js
// exitcode: C0000420
// stdout:
// 
// stderr:
// ASSERTION 5728: (inetcore\jscript\lib\backend\globopt.cpp, line 17845) !canStoreTemp || instr->dstIsTempObject
//  Failure: (!canStoreTemp || instr->dstIsTempObject)
// FATAL ERROR: jshost.exe failed due to exception code c0000420
// 
// 
// 

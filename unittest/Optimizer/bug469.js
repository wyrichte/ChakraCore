//FileName: \\bptweb02\users\runtimetoolsbugs\casanalyzed\exprgen\fbl_ie_stage_dev3\150420-1820\x86\nebularun_x86\201504220412425151\ddltd1_bff740d5-3d7b-4ddf-8474-03f7995b9307\bug0.js
//Configuration: lessmath_variableMRU.xml
//Testcase Number: 10687
//Switches: -PrintSystemException   -maxinterpretcount:1 -maxsimplejitruncount:1 -werexceptionsupport  -MinSwitchJumpTableSize:3 -bgjit- -loopinterpretcount:1 -forcedeferparse -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -off:polymorphicinlinecache -force:fieldcopyprop -force:fieldhoist -sse:2 -force:inline -force:interpreterautoprofile -ForceArrayBTree -force:rejit -off:DelayCapture -force:ScriptFunctionWithInlineCache -force:atom -off:ArrayCheckHoist -off:lossyinttypespec -force:fixdataprops -off:fefixedmethods -off:checkthis -ValidateInlineStack -off:aggressiveinttypespec -off:trackintusage -off:ParallelParse -off:bailonnoprofile -off:BoundCheckElimination -off:NativeArray -off:ArrayLengthHoist
//Baseline Switches: -nonative -werexceptionsupport  -PrintSystemException
//Arch: X86
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 150420-1820
//FullBuild: 10059.0.150420
//MachineName: BPT36967
//InstructionSet: 
//reduced switches: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile
//noRepro switches1: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile -off:InterpreterAutoProfile
//noRepro switches2: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile -off:InterpreterProfile
//noRepro switches3: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile -off:JITLoopBody
//noRepro switches4: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile -off:DynamicProfile
//noRepro switches5: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile -off:UseFixedDataProps
//noRepro switches6: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile -off:TypeSpec
//noRepro switches7: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile -off:Inline
//noRepro switches8: -loopinterpretcount:1 -bgjit- -maxsimplejitruncount:1 -maxinterpretcount:1 -forcedeferparse -off:aggressiveinttypespec -off:bailonnoprofile -off:DeferParse
var protoObj0 = {};
var obj1 = {};
var VarArr0 = [
    protoObj0,
    -1
  ];
protoObj0.prop1 = -314917159;
function v8() {
  for (var _strvar0 of VarArr0) {
    if (!protoObj0) {
      if (FloatArr0.push(protoObj0.prop1)) {
        arrObj0.prop1 = arrObj0.prop1.substring();
      }
    }
    obj1 * (obj1 - protoObj0.prop1);
    for (var _strvar0 in VarArr0) {
      if (_strvar0.indexOf()) {
      }
    }
  }
}
v8();

WScript.Echo("Pass");

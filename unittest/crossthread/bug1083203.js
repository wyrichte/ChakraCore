//Configuration: cctx.xml
//Testcase Number: 14744
//Switches: -PrintSystemException  -CheckMemoryLeak-   -maxinterpretcount:1 -maxsimplejitruncount:2 -werexceptionsupport  -force:fieldhoist -forcejitloopbody -force:fieldcopyprop -MinSwitchJumpTableSize:3 -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -force:polymorphicinlinecache -forcedeferparse -force:inline -off:ArrayCheckHoist -force:atom -off:checkthis -off:NativeArray -force:rejit -off:aggressiveinttypespec -force:fixdataprops -force:ScriptFunctionWithInlineCache -off:fefixedmethods -ForceArrayBTree -off:lossyinttypespec -off:trackintusage -ValidateInlineStack -off:ParallelParse -off:DelayCapture -off:BoundCheckElimination -off:LoopCountBasedBoundCheckHoist -off:BoundCheckHoist -off:ArrayLengthHoist -off:bailonnoprofile -off:floattypespec -off:TypedArrayTypeSpec -off:EliminateArrayAccessHelperCall
//Baseline Switches: -nonative -werexceptionsupport  -PrintSystemException  -CheckMemoryLeak-
//Arch: AMD64
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 141114-2030
//FullBuild: 9885.0.141114
//MachineName: BPT33450
//InstructionSet: 
//reduced switches: -maxsimplejitruncount:2 -maxinterpretcount:1 -off:nativearray
function test0() {
  function makeArrayLength() {
    return 100;
  }
  var obj0 = {};
  var obj1 = {};
  var litObj0 = {};
  var func1 = function () {
    var __loopvar2 = 6;
    for (; protoObj0; protoObj0) {
      __loopvar2 += 4;
      if (__loopvar2 === 6 + 16) {
        break;
      }
      for (var _strvar0 in ary) {
      }
      var sc1 = WScript.LoadScriptFile('DummyFileForCctx.js', 'samethread');
      sc1.litObj0 = litObj0;
      sc1.litObj1 = protoObj1;
      var sc1_cctx = sc1.Debug.parseFunction('var fPolyProp = function (o) {\n  if (o!==undefined) {\n    WScript.Echo(o.prop0 + \' \' + o.prop1 + \' \' + o.prop2);\n  }\n};\nfPolyProp(litObj0); \nfPolyProp(litObj1); \n;\n;\n      ');
      sc1_cctx();
    }
  };
  obj0.method1 = func1;
  obj1.method0 = obj0.method1;
  var ary = new Array();
  ary[ary.length] = 1;
  ary.length = makeArrayLength();
  protoObj0 = Object();
  protoObj1 = Object(obj1);
  for (var _strvar0 in ary) {
    var uniqobj16 = [protoObj1];
    var uniqobj17 = uniqobj16[0];
    uniqobj17.method0();
    var v0 = {
        v1: function () {
          return function bar() {
          };
        }
      };
    obj0.v4 = v0.v1();
    var v20 = obj0.v4(ary.push(protoObj1.prop0 = ary.reverse()));
  }
}
test0();
Debug.setAutoProxyName("test0");
test0();

// === Output ===
// command: D:\CAS-Exprgen-AMD64-3\JsReducer\JSHost.exe -maxsimplejitruncount:2 -maxinterpretcount:1 -off:nativearray step1191.js
// exitcode: C0000420
// stdout:
// undefined undefined undefined
// undefined undefined undefined
// undefined undefined undefined
// undefined undefined undefined
// undefined undefined undefined
// undefined undefined undefined
// undefined undefined undefined
// ,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,1, undefined undefined
// 
// stderr:
// ASSERTION 1144: (inetcore\jscript\lib\runtime\Library\ES5ArrayEnumerator.h, line 20) Derived class need to define marshal to script context
//  Failure: (VirtualTableInfo<ES5ArrayEnumerator>::HasVirtualTable(this))
// FATAL ERROR: jshost.exe failed due to exception code c0000420
// 
// 
// 

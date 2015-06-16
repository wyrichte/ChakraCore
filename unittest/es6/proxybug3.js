//Configuration: es6.xml
//Testcase Number: 2407
//Switches: -PrintSystemException   -maxinterpretcount:3 -maxsimplejitruncount:6 -werexceptionsupport  -forcejitloopbody -force:rejit -force:ScriptFunctionWithInlineCache -force:fixdataprops -force:atom -ForceArrayBTree -off:lossyinttypespec -off:trackintusage -off:ParallelParse -off:fefixedmethods -off:LoopCountBasedBoundCheckHoist
//Baseline Switches: -nonative -werexceptionsupport  -PrintSystemException
//Arch: X86
//Flavor: chk
//Branch: fbl_ie_stage_dev3
//Build: 150425-1820
//FullBuild: 10102.0.150425
//MachineName: BPT42065
//InstructionSet: 
//reduced switches: -maxsimplejitruncount:6 -maxinterpretcount:3
var arrObj0 = {};
var func4 = function () {
  arrObj0 = new Proxy(arrObj0, {});
};
for (i =0; i < 2; i++) {
  function v0() {
  }
  v0.prototype = arrObj0;
  var v1 = new v0();
  var test = { prop4: func4() };
}
WScript.Echo("PASS");

// === Output ===
// command: D:\CAS-Exprgen-X86-3\JsReducer\JSHost.exe -maxsimplejitruncount:6 -maxinterpretcount:3 step632.js
// exitcode: C0000420
// stdout:
// [object Object]
// 
// stderr:
// ASSERTION 6748: (inetcore\jscript\lib\runtime\language\javascriptoperators.cpp, line 5571) type->GetPrototype() == JavascriptOperators::GetPrototypeObjectForConstructorCache(constructor, requestContext, cachedProtoCanBeCached)
//  Failure: (type->GetPrototype() == JavascriptOperators::GetPrototypeObjectForConstructorCache(constructor, requestContext, cachedProtoCanBeCached))
// FATAL ERROR: jshost.exe failed due to exception code c0000420
// 
// 
// 

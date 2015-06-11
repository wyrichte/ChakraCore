var __counter = 0;
function test0() {
}
shouldBailout = true;
var loopInvariant = 0;
var GiantPrintArray = [];
__counter++;
function leaf() {
}
var obj0 = {};
var obj1 = {};
var protoObj1 = {};
var arrObj0 = {};
var func0 = function (argArr0, argFunc1, argArrObj2) {
  var sc0Code = 'GiantPrintArray.push(\'a = \' + (a|0));\nif(((protoObj0.prop0, -125, (++ protoObj1.prop0)) >= (test0.caller))) {\n  function func5 () {\n    this.prop0 = 4.9473844804737E+18;\n  }\n  var uniqobj0 = new func5();\n  var __loopvar4 = loopInvariant - 12;\n  LABEL0: \n  LABEL1: \n  for(; a < (a);__loopvar4 += 4 , a++) {\n    if (__loopvar4 > loopInvariant) break;\n    GiantPrintArray.push(\'obj1.length = \' + (obj1.length|0));\n    break ;\n    b =(protoObj0.prop0 >> 624954924);\n    var uniqobj1 = Object.create(arrObj0);\n  }\n  if(((((new RangeError()) instanceof ((typeof Function == \'function\' ) ? Function : Object)) * ((typeof(5.84421250662406E+18)  == null)  * (uniqobj0.prop0 ? obj1.length : 673864516) + i8[(arrObj0.length) & 255]) + 4) == ((argArr0.push((shouldBailout ? (Object.defineProperty(protoObj0, \'prop0\', {get: function() { WScript.Echo(\'protoObj0.prop0 getter\'); return 3; }, set: function(_x) { WScript.Echo(\'protoObj0.prop0 setter\'); }, configurable: true}), -4.60748819966749E+17) : -4.60748819966749E+17), argArrObj2[(((-743003424 >= 0 ? -743003424 : 0)) & 0XF)], (/a/ instanceof ((typeof String == \'function\' ) ? String : Object)), (test0.caller),   (shouldBailout ? argFunc1() : argFunc1()), (argArr0.pop()), ui8[(567477603) & 255]))\n != ((new Error(\'abc\')) instanceof ((typeof argFunc1 == \'function\' ) ? argFunc1 : Object))))) {\n    c = (84 in f64);\n    e =((c <= a) * (+undefined + b));\n  }\n  else {\n    c /=argFunc1();\n    obj7 = obj1;\n  }\n  var __loopvar4 = loopInvariant;\n  for(; arrObj0.length < (  (shouldBailout ? argFunc1() : argFunc1())); arrObj0.length++) {\n    if (__loopvar4 >= loopInvariant + 2) break;\n    __loopvar4++;\n    var r = arrObj0.length;\n    GiantPrintArray.push(\'obj0.prop0 = \' + (obj0.prop0|0));\n    obj0.prop0 = argArr0[(((((shouldBailout ? (argArr0[((((test0.caller)) >= 0 ? ( (test0.caller)) : 0) & 0xF)] = \'x\') : undefined ), (test0.caller)) >= 0 ? (test0.caller) : 0)) & 0XF)];\n  }\n  argFunc1.call(arrObj0 );\n}\nelse {\n  var uniqobj2 = Object.create(obj1);\n  arrObj0.prop0 = ((new Array()) instanceof ((typeof Function == \'function\' ) ? Function : Object));\n}\n;\n    ';
  var sc0 = WScript.LoadScriptFile('dummy_cctx.js');
  sc0.argArr0 = argArr0;
  sc0.argArrObj2 = argArrObj2;
  sc0.argFunc1 = argFunc1;
  var sc0_cctx = Debug.parseFunction(sc0Code);
  sc0_cctx();
};
var func3 = function () {
  return func0.call(protoObj1, ary, leaf, arrObj0);
};
var func4 = function (argObj15) {
  for (; argObj15.prop0 < ary[shouldBailout ? ary[6] = 'x' : undefined, 6]; true instanceof (typeof Function == 'function' ? Function : Object)) {
  }
};
obj0.method1 = func3;
obj1.method1 = func4;
var ary = Array();
var i8 = Int8Array();
var ui8 = Uint8Array();
var a = -2147483647;
var c = -5678175356252110000;
protoObj0 = Object(obj0);
protoObj1.prop0 = 535805939189958000;
var sc4Code = 'protoObj0.prop0 *=obj0.length;\nvar __loopvar1 = loopInvariant;\nfor(;;) {\n  __loopvar1 += 4;\n  if (__loopvar1 >= loopInvariant + 12) break;\n  protoObj0.method1((obj0.prop0 < __loopvar1),arrObj0);\n  var q = (protoObj1.length++ );\n  obj6 = Object.create(protoObj0);\n  var __loopvar2 = loopInvariant - 9,__loopSecondaryVar2_0 = loopInvariant - 9,__loopSecondaryVar2_1 = loopInvariant;\n  LABEL0: \n  do {\n    __loopvar2 += 3;\n    __loopSecondaryVar2_1++;\n    if (__loopvar2 > loopInvariant + 3) break;\n    __loopSecondaryVar2_0 += 3;\n    var uniqobj8 = [protoObj1, obj1];\n    var uniqobj9 = uniqobj8[__counter%uniqobj8.length];\n    uniqobj9.method1(obj6);\n    obj0 = obj1;\n    arrObj0.prop0 = ary[((shouldBailout ? (ary[__loopSecondaryVar2_1 + 1] = \'x\') : undefined ), __loopSecondaryVar2_1 + 1)];\n  } while((Object.create(protoObj1)))\n  var __loopvar2 = loopInvariant,__loopSecondaryVar2_0 = loopInvariant,__loopSecondaryVar2_1 = loopInvariant - 9;\n  LABEL0: \n  LABEL1: \n  while((protoObj1.method1(obj6))) {\n    __loopSecondaryVar2_1 += 3;\n    if (__loopvar2 == loopInvariant + 3) break;\n    __loopvar2++;\n    __loopSecondaryVar2_0 += 4;\n  }\n}\nvar uniqobj10 = [protoObj1, obj1, protoObj0];\nuniqobj10[__counter%uniqobj10.length].method0(protoObj0);\nb = 224;\n;\n  ';
var sc4 = WScript.LoadScriptFile('dummy_cctx.js', 'samethread');
sc4.protoObj0 = protoObj0;
sc4.obj0 = obj0;
sc4.arrObj0 = arrObj0;
sc4.protoObj1 = protoObj1;
sc4.obj1 = obj1;
sc4.ary = ary;
sc4.shouldBailout = shouldBailout;
sc4.__counter = __counter;
sc4.loopInvariant = loopInvariant;
var sc4_cctx = sc4.Debug.parseFunction(sc4Code);
sc4_cctx();
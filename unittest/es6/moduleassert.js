//Configuration: configs\es6.xml
//Testcase Number: 5367
//Switches: -PrintSystemException  -es6module -es6generators  -maxinterpretcount:5 -maxsimplejitruncount:3 -MinMemOpCount:0 -werexceptionsupport  -forcedeferparse -MaxLinearStringCaseCount:2 -MaxLinearIntCaseCount:2 -MinSwitchJumpTableSize:2 -force:fieldcopyprop -bgjit- -loopinterpretcount:1 -force:polymorphicinlinecache -off:stackargopt
//Baseline Switches: -nonative -werexceptionsupport  -PrintSystemException  -es6module -es6generators
//Arch: x86
//Flavor: test
//BuildName: full
//BuildRun: 160711_1211
//BuildId: 00005.64049
//BuildHash: dac2b7dce2bbe656037fd99fbb2cf2fe53a20d11
//BinaryPath: \\chakrafs\fs\builds\chakrafull\unreleased\rs2\1607\00005.64049_160711.1211_akroshg_dac2b7dce2bbe656037fd99fbb2cf2fe53a20d11\bin\x86_test
//MachineName: BPT15921
//InstructionSet: 
//reduced switches: -es6module -maxinterpretcount:5 -maxsimplejitruncount:3 -bgjit- -loopinterpretcount:1
WScript.RegisterModuleSource('module0_af60f209-dff4-41e2-8a11-0206e428c198.js', `
var module0_localbinding_0 = Infinity;
export { module0_localbinding_0 as default };
`);
WScript.RegisterModuleSource('module1_afa96955-9820-4822-9b34-005434e9a93e.js', `
var loopInvariant = shouldBailout ? 10 : 3;
var GiantPrintArray = [];
__counter++;;
function makeArrayLength(x) { if(x < 1 || x > 4294967295 || x != x || isNaN(x) || !isFinite(x)) return 100; else return Math.floor(x) & 0xffff; };;
function leaf() { return 100; };
class module2BaseClass { };;
var obj0 = {};
var obj1 = {};
var protoObj1 = {};
var arrObj0 = {};
var litObj0 = {prop1: 3.14159265358979};
var litObj1 = {prop0: 0, prop1: 1};
var arrObj0 = {};
var func0 = function(){
  var __loopvar3 = loopInvariant - 11,__loopSecondaryVar3_0 = loopInvariant - 15,__loopSecondaryVar3_1 = loopInvariant - 15;
  LABEL0: 
  LABEL1: 
  for(; obj1.prop1 < (((new RegExp('xyz')) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object))); -20) {
    if (__loopvar3 >= loopInvariant + -3) break;
    __loopvar3 += 3;
    __loopSecondaryVar3_0 += 4;
    __loopSecondaryVar3_1 += 4;
    return 272667339;
    if(shouldBailout){
      return  'somestring'
    }
    var strvar9 = 'ÉvB!!' + '!F-(:19x';
    strvar9 = strvar9.substring((strvar9.length)/3,(strvar9.length)/2);
  }
  var reResult0=/\s\b\S+|a/iy.test('-');
  if((arrObj0.prop1 - 109)) {
    if(shouldBailout){
      return  'somestring'
    }
    var strvar9 = ('(L'+'Ø)'+'!5' + 'Æ+' + arrObj0[(14)]);
    strvar9 = strvar9.substring((strvar9.length)/1,(strvar9.length)/3);
    litObj1 = litObj0;
    strvar9 = 'Á' + '?t-¡' + (typeof f64[1637432166]);
    reResult0 |=arrObj0[(14)];
    if(shouldBailout){
      return  'somestring'
    }
  }
  else {
    reResult0 = (c <= g);
    reResult0 >>=((h > c) == ((~ f) * (c <= g) + e));
    protoObj1.prop0 &=(typeof(g)  != 'object') ;
    protoObj1.prop1 =(obj0.prop0++ );
  }
  return ui32[(((((new RegExp('xyz')) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)) % (ary.shift())) / (2 == 0 ? 1 : 2))) & 255];
};
var func1 = function(){
  aliasOfarrObj0.prop5=func0;
  //Snippet 1: basic inlining test
  aliasOfarrObj0.prop1 = (function(x,y,z) {
    WScript.Echo(strvar6 <=func0.call(aliasOfarrObj0 ));
    GiantPrintArray.push("Snippet 1: ",x,y,z);
    return obj0.prop1;
  })('caller',2094650667,(new module2BaseClass()));
  
  arrObj0 = protoObj1;
  return (d < arrObj0.prop1);
};
var func2 = function(argMath92 = i16[((new module2BaseClass())) & 255],argMath93,argMath94){
  argMath94 =(i16[(245) & 255] << ('z'.indexOf((')J7!i' + 'Í#ûÒ%#ºW').replace(strvar0, '!' + 'A_g!'))));
  return 'caller';
};
var func3 = function(){
  return (new module2BaseClass());
};
var func4 = function(){
  return (new module2BaseClass());
};
obj0.method0 = func2;
obj0.method1 = func2;
obj1.method0 = func0;
obj1.method1 = func0;
arrObj0.method0 = func1;
arrObj0.method1 = arrObj0.method0;
var ary = new Array(10);
var i8 = new Int8Array(256);
var i16 = new Int16Array(256);
var i32 = new Int32Array(256);
var ui8 = new Uint8Array(256);
var ui16 = new Uint16Array(256);
var ui32 = new Uint32Array(256);
var f32 = new Float32Array(256);
var f64 = new Float64Array(256);
var uic8 = new Uint8ClampedArray(256);
var IntArr0 = new Array(-178136927,-2147483648,7534139964225440768,-4963918594748139520,1533757193,1799681143,8840318718834587648,1448270212,-1,660603117,2);
var IntArr1 = [];
var FloatArr0 = new Array(4294967295,-1073741824,784462959.1,-2032875814.9,5.90161368352803E+18,-1390318425.9,-829306065.9,2.40266623347532E+18,4294967295);
var VarArr0 = [strvar0,1510038175,-205,160,519740633,-1195470694.9,7.85249116055337E+18,8.13049086053444E+18,-258093103,-1073741824,-248884281,113568649,-468692021,-2147483647,7];
var a = 48380171.1;
var b = 1158791551;
var c = 1048511885;
var d = -312809819;
var e = 234590590;
var f = 2132549253.1;
var g = -2;
var h = 60;
var strvar0 = '%$i(+' + '!§=##(##';
var strvar1 = 'ÉvB!!' + '!F-(:19x';
var strvar2 = '%' + 'h$St';
var strvar3 = '+#!_1' + 'XDjs;()Í';
var strvar4 = 'æ$'+'7,'+'á#' + '$!';
var strvar5 = '!%pÃ,' + '#!î%%#,î';
var strvar6 = ')J7!i' + 'Í#ûÒ%#ºW';
var strvar7 = 'N)'+'%+'+'ÐZ' + '§S';
arrObj0[0] = 263968257;
arrObj0[1] = 4.15291702379642E+17;
arrObj0[2] = -1344610374.9;
arrObj0[3] = -5.30371754387382E+17;
arrObj0[4] = -130;
arrObj0[5] = 2.3477884534906E+18;
arrObj0[6] = 5.0137674970111E+18;
arrObj0[7] = 1071234454;
arrObj0[8] = -5.35218855961021E+18;
arrObj0[9] = 1688713044.1;
arrObj0[10] = 3.67178969203058E+18;
arrObj0[11] = 0;
arrObj0[12] = -28627365;
arrObj0[13] = 4.72216761538867E+18;
arrObj0[14] = -7.06362519892309E+18;
arrObj0[arrObj0.length-1] = 715822797;
arrObj0.length = makeArrayLength(-198);
ary[0] = 157374197;
ary[1] = -66;
ary[2] = 0;
ary[3] = -483385338;
ary[4] = -1608023752;
ary[5] = 70;
ary[6] = -3.97472095523329E+18;
ary[7] = -1.18431780644445E+18;
ary[8] = -228;
ary[9] = 1530792508;
ary[10] = -1028501809;
ary[11] = -1.02815779202395E+18;
ary[12] = 1680643706;
ary[13] = 1762855713;
ary[14] = -1073741824;
ary[ary.length-1] = -488382331;
ary.length = makeArrayLength(9673240);
var protoObj1 = Object.create(obj1);
var aliasOfarrObj0 = arrObj0;
var aliasOfFloatArr0 = FloatArr0;;
var aliasOfi8 = i8;;
obj0.prop0 = -3.14387752038325E+18;
obj0.prop1 = 1067318894;
obj0.length = makeArrayLength(-8.98379822515593E+18);
obj1.prop0 = -678107569.9;
obj1.prop1 = 445851436.1;
obj1.length = makeArrayLength(-202);
protoObj1.prop0 = -186583034;
protoObj1.prop1 = 2;
protoObj1.length = makeArrayLength(-1709642951);
arrObj0.prop0 = 3;
arrObj0.prop1 = -1505625434.9;
arrObj0.length = makeArrayLength(1427241621.1);
aliasOfarrObj0.prop0 = 756060752.1;
aliasOfarrObj0.prop1 = 1;
aliasOfarrObj0.length = makeArrayLength(-196);
IntArr1[0] = -110;
IntArr1[2] = 12;
IntArr1[3] = 1894564480;
IntArr1[1] = -2;
IntArr1[4] = -1216202745;
FloatArr0[9] = 1.40827950345691E+18;
FloatArr0[11] = -30;
FloatArr0[10] = -1740567964.9;
import {  } from 'module0_af60f209-dff4-41e2-8a11-0206e428c198.js';
export default function(){
}
obj1.method0();
function func5 () {
}
var uniqobj12 = new func5();
(Object.defineProperty(arrObj0, 'prop0', {writable: true, enumerable: false, configurable: true }));
arrObj0.prop0 = f;
protoObj1.prop5=(((aliasOfarrObj0.prop0 == d) | ('%' + 'h$St'.indexOf(strvar6))) + obj0.method0.call(obj0 , uic8[(176) & 255], 'caller', ((aliasOfarrObj0.prop0 += ui16[(81) & 255]) * ((new module2BaseClass()) + ui16[(179) & 255]))));
function func6 (arg0) {
  this.prop0 = arg0;
}
var uniqobj13 = new func6(obj1.method1.call(aliasOfarrObj0 ));
WScript.Echo('a = ' + (a|0));
WScript.Echo('b = ' + (b|0));
WScript.Echo('c = ' + (c|0));
WScript.Echo('d = ' + (d|0));
WScript.Echo('e = ' + (e|0));
WScript.Echo('f = ' + (f|0));
WScript.Echo('g = ' + (g|0));
WScript.Echo('h = ' + (h|0));
WScript.Echo('obj0.prop0 = ' + (obj0.prop0|0));
WScript.Echo('obj0.prop1 = ' + (obj0.prop1|0));
WScript.Echo('obj0.length = ' + (obj0.length|0));
WScript.Echo('obj1.prop0 = ' + (obj1.prop0|0));
WScript.Echo('obj1.prop1 = ' + (obj1.prop1|0));
WScript.Echo('obj1.length = ' + (obj1.length|0));
WScript.Echo('protoObj1.prop0 = ' + (protoObj1.prop0|0));
WScript.Echo('protoObj1.prop1 = ' + (protoObj1.prop1|0));
WScript.Echo('protoObj1.length = ' + (protoObj1.length|0));
WScript.Echo('arrObj0.prop0 = ' + (arrObj0.prop0|0));
WScript.Echo('arrObj0.prop1 = ' + (arrObj0.prop1|0));
WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
WScript.Echo('aliasOfarrObj0.prop0 = ' + (aliasOfarrObj0.prop0|0));
WScript.Echo('aliasOfarrObj0.prop1 = ' + (aliasOfarrObj0.prop1|0));
WScript.Echo('aliasOfarrObj0.length = ' + (aliasOfarrObj0.length|0));
WScript.Echo('protoObj1.prop5 = ' + (protoObj1.prop5|0));
WScript.Echo('uniqobj13.prop0 = ' + (uniqobj13.prop0|0));
WScript.Echo('strvar0 = ' + (strvar0));
WScript.Echo('strvar1 = ' + (strvar1));
WScript.Echo('strvar2 = ' + (strvar2));
WScript.Echo('strvar3 = ' + (strvar3));
WScript.Echo('strvar4 = ' + (strvar4));
WScript.Echo('strvar5 = ' + (strvar5));
WScript.Echo('strvar6 = ' + (strvar6));
WScript.Echo('strvar7 = ' + (strvar7));
WScript.Echo('arrObj0[0] = ' + (arrObj0[0]|0));
WScript.Echo('arrObj0[1] = ' + (arrObj0[1]|0));
WScript.Echo('arrObj0[2] = ' + (arrObj0[2]|0));
WScript.Echo('arrObj0[3] = ' + (arrObj0[3]|0));
WScript.Echo('arrObj0[4] = ' + (arrObj0[4]|0));
WScript.Echo('arrObj0[5] = ' + (arrObj0[5]|0));
WScript.Echo('arrObj0[6] = ' + (arrObj0[6]|0));
WScript.Echo('arrObj0[7] = ' + (arrObj0[7]|0));
WScript.Echo('arrObj0[8] = ' + (arrObj0[8]|0));
WScript.Echo('arrObj0[9] = ' + (arrObj0[9]|0));
WScript.Echo('arrObj0[10] = ' + (arrObj0[10]|0));
WScript.Echo('arrObj0[11] = ' + (arrObj0[11]|0));
WScript.Echo('arrObj0[12] = ' + (arrObj0[12]|0));
WScript.Echo('arrObj0[13] = ' + (arrObj0[13]|0));
WScript.Echo('arrObj0[14] = ' + (arrObj0[14]|0));
WScript.Echo('arrObj0[arrObj0.length-1] = ' + (arrObj0[arrObj0.length-1]|0));
WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
WScript.Echo('ary[0] = ' + (ary[0]|0));
WScript.Echo('ary[1] = ' + (ary[1]|0));
WScript.Echo('ary[2] = ' + (ary[2]|0));
WScript.Echo('ary[3] = ' + (ary[3]|0));
WScript.Echo('ary[4] = ' + (ary[4]|0));
WScript.Echo('ary[5] = ' + (ary[5]|0));
WScript.Echo('ary[6] = ' + (ary[6]|0));
WScript.Echo('ary[7] = ' + (ary[7]|0));
WScript.Echo('ary[8] = ' + (ary[8]|0));
WScript.Echo('ary[9] = ' + (ary[9]|0));
WScript.Echo('ary[10] = ' + (ary[10]|0));
WScript.Echo('ary[11] = ' + (ary[11]|0));
WScript.Echo('ary[12] = ' + (ary[12]|0));
WScript.Echo('ary[13] = ' + (ary[13]|0));
WScript.Echo('ary[14] = ' + (ary[14]|0));
WScript.Echo('ary[ary.length-1] = ' + (ary[ary.length-1]|0));
WScript.Echo('ary.length = ' + (ary.length|0));
for (var i = 0; i < GiantPrintArray.length; i++) {
  WScript.Echo(GiantPrintArray[i]);
  }
;
WScript.Echo('sumOfary = ' +  ary.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_ary = ' +  ary.slice(0, 11));;
WScript.Echo('sumOfIntArr0 = ' +  IntArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_IntArr0 = ' +  IntArr0.slice(0, 11));;
WScript.Echo('sumOfIntArr1 = ' +  IntArr1.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_IntArr1 = ' +  IntArr1.slice(0, 11));;
WScript.Echo('sumOfFloatArr0 = ' +  FloatArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_FloatArr0 = ' +  FloatArr0.slice(0, 11));;
WScript.Echo('sumOfVarArr0 = ' +  VarArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_VarArr0 = ' +  VarArr0.slice(0, 11));;
WScript.Echo('sumOfaliasOfFloatArr0 = ' +  aliasOfFloatArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_aliasOfFloatArr0 = ' +  aliasOfFloatArr0.slice(0, 11));;
`);

WScript.RegisterModuleSource('module2_4c1abe23-d6d6-48ac-9650-608cb0c4a32d.js', `
var loopInvariant = shouldBailout ? 10 : 3;
var GiantPrintArray = [];
__counter++;;
function makeArrayLength(x) { if(x < 1 || x > 4294967295 || x != x || isNaN(x) || !isFinite(x)) return 100; else return Math.floor(x) & 0xffff; };;
function leaf() { return 100; };
class module2BaseClass { };;
var obj0 = {};
var obj1 = {};
var protoObj1 = {};
var arrObj0 = {};
var litObj0 = {prop1: 3.14159265358979};
var litObj1 = {prop0: 0, prop1: 1};
var arrObj0 = {};
var func0 = function(){
  var __loopvar3 = loopInvariant - 11,__loopSecondaryVar3_0 = loopInvariant - 15,__loopSecondaryVar3_1 = loopInvariant - 15;
  LABEL0: 
  LABEL1: 
  for(; obj1.prop1 < (((new RegExp('xyz')) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object))); -20) {
    if (__loopvar3 >= loopInvariant + -3) break;
    __loopvar3 += 3;
    __loopSecondaryVar3_0 += 4;
    __loopSecondaryVar3_1 += 4;
    return 272667339;
    if(shouldBailout){
      return  'somestring'
    }
    var strvar9 = 'ÉvB!!' + '!F-(:19x';
    strvar9 = strvar9.substring((strvar9.length)/3,(strvar9.length)/2);
  }
  var reResult0=/\s\b\S+|a/iy.test('-');
  if((arrObj0.prop1 - 109)) {
    if(shouldBailout){
      return  'somestring'
    }
    var strvar9 = ('(L'+'Ø)'+'!5' + 'Æ+' + arrObj0[(14)]);
    strvar9 = strvar9.substring((strvar9.length)/1,(strvar9.length)/3);
    litObj1 = litObj0;
    strvar9 = 'Á' + '?t-¡' + (typeof f64[1637432166]);
    reResult0 |=arrObj0[(14)];
    if(shouldBailout){
      return  'somestring'
    }
  }
  else {
    reResult0 = (c <= g);
    reResult0 >>=((h > c) == ((~ f) * (c <= g) + e));
    protoObj1.prop0 &=(typeof(g)  != 'object') ;
    protoObj1.prop1 =(obj0.prop0++ );
  }
  return ui32[(((((new RegExp('xyz')) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)) % (ary.shift())) / (2 == 0 ? 1 : 2))) & 255];
};
var func1 = function(){
  aliasOfarrObj0.prop5=func0;
  //Snippet 1: basic inlining test
  aliasOfarrObj0.prop1 = (function(x,y,z) {
    WScript.Echo(strvar6 <=func0.call(aliasOfarrObj0 ));
    GiantPrintArray.push("Snippet 1: ",x,y,z);
    return obj0.prop1;
  })('caller',2094650667,(new module2BaseClass()));
  
  arrObj0 = protoObj1;
  return (d < arrObj0.prop1);
};
var func2 = function(argMath92 = i16[((new module2BaseClass())) & 255],argMath93,argMath94){
  argMath94 =(i16[(245) & 255] << ('z'.indexOf((')J7!i' + 'Í#ûÒ%#ºW').replace(strvar0, '!' + 'A_g!'))));
  return 'caller';
};
var func3 = function(){
  return (new module2BaseClass());
};
var func4 = function(){
  return (new module2BaseClass());
};
obj0.method0 = func2;
obj0.method1 = func2;
obj1.method0 = func0;
obj1.method1 = func0;
arrObj0.method0 = func1;
arrObj0.method1 = arrObj0.method0;
var ary = new Array(10);
var i8 = new Int8Array(256);
var i16 = new Int16Array(256);
var i32 = new Int32Array(256);
var ui8 = new Uint8Array(256);
var ui16 = new Uint16Array(256);
var ui32 = new Uint32Array(256);
var f32 = new Float32Array(256);
var f64 = new Float64Array(256);
var uic8 = new Uint8ClampedArray(256);
var IntArr0 = new Array(-178136927,-2147483648,7534139964225440768,-4963918594748139520,1533757193,1799681143,8840318718834587648,1448270212,-1,660603117,2);
var IntArr1 = [];
var FloatArr0 = new Array(4294967295,-1073741824,784462959.1,-2032875814.9,5.90161368352803E+18,-1390318425.9,-829306065.9,2.40266623347532E+18,4294967295);
var VarArr0 = [strvar0,1510038175,-205,160,519740633,-1195470694.9,7.85249116055337E+18,8.13049086053444E+18,-258093103,-1073741824,-248884281,113568649,-468692021,-2147483647,7];
var a = 48380171.1;
var b = 1158791551;
var c = 1048511885;
var d = -312809819;
var e = 234590590;
var f = 2132549253.1;
var g = -2;
var h = 60;
var strvar0 = '%$i(+' + '!§=##(##';
var strvar1 = 'ÉvB!!' + '!F-(:19x';
var strvar2 = '%' + 'h$St';
var strvar3 = '+#!_1' + 'XDjs;()Í';
var strvar4 = 'æ$'+'7,'+'á#' + '$!';
var strvar5 = '!%pÃ,' + '#!î%%#,î';
var strvar6 = ')J7!i' + 'Í#ûÒ%#ºW';
var strvar7 = 'N)'+'%+'+'ÐZ' + '§S';
arrObj0[0] = 263968257;
arrObj0[1] = 4.15291702379642E+17;
arrObj0[2] = -1344610374.9;
arrObj0[3] = -5.30371754387382E+17;
arrObj0[4] = -130;
arrObj0[5] = 2.3477884534906E+18;
arrObj0[6] = 5.0137674970111E+18;
arrObj0[7] = 1071234454;
arrObj0[8] = -5.35218855961021E+18;
arrObj0[9] = 1688713044.1;
arrObj0[10] = 3.67178969203058E+18;
arrObj0[11] = 0;
arrObj0[12] = -28627365;
arrObj0[13] = 4.72216761538867E+18;
arrObj0[14] = -7.06362519892309E+18;
arrObj0[arrObj0.length-1] = 715822797;
arrObj0.length = makeArrayLength(-198);
ary[0] = 157374197;
ary[1] = -66;
ary[2] = 0;
ary[3] = -483385338;
ary[4] = -1608023752;
ary[5] = 70;
ary[6] = -3.97472095523329E+18;
ary[7] = -1.18431780644445E+18;
ary[8] = -228;
ary[9] = 1530792508;
ary[10] = -1028501809;
ary[11] = -1.02815779202395E+18;
ary[12] = 1680643706;
ary[13] = 1762855713;
ary[14] = -1073741824;
ary[ary.length-1] = -488382331;
ary.length = makeArrayLength(9673240);
var protoObj1 = Object.create(obj1);
var aliasOfarrObj0 = arrObj0;
var aliasOfFloatArr0 = FloatArr0;;
var aliasOfi8 = i8;;
obj0.prop0 = -3.14387752038325E+18;
obj0.prop1 = 1067318894;
obj0.length = makeArrayLength(-8.98379822515593E+18);
obj1.prop0 = -678107569.9;
obj1.prop1 = 445851436.1;
obj1.length = makeArrayLength(-202);
protoObj1.prop0 = -186583034;
protoObj1.prop1 = 2;
protoObj1.length = makeArrayLength(-1709642951);
arrObj0.prop0 = 3;
arrObj0.prop1 = -1505625434.9;
arrObj0.length = makeArrayLength(1427241621.1);
aliasOfarrObj0.prop0 = 756060752.1;
aliasOfarrObj0.prop1 = 1;
aliasOfarrObj0.length = makeArrayLength(-196);
IntArr1[0] = -110;
IntArr1[2] = 12;
IntArr1[3] = 1894564480;
IntArr1[1] = -2;
IntArr1[4] = -1216202745;
FloatArr0[9] = 1.40827950345691E+18;
FloatArr0[11] = -30;
FloatArr0[10] = -1740567964.9;
import {  } from 'module0_af60f209-dff4-41e2-8a11-0206e428c198.js';
export default function(){
}
obj1.method0();
function func5 () {
}
var uniqobj12 = new func5();
(Object.defineProperty(arrObj0, 'prop0', {writable: true, enumerable: false, configurable: true }));
arrObj0.prop0 = f;
protoObj1.prop5=(((aliasOfarrObj0.prop0 == d) | ('%' + 'h$St'.indexOf(strvar6))) + obj0.method0.call(obj0 , uic8[(176) & 255], 'caller', ((aliasOfarrObj0.prop0 += ui16[(81) & 255]) * ((new module2BaseClass()) + ui16[(179) & 255]))));
function func6 (arg0) {
  this.prop0 = arg0;
}
var uniqobj13 = new func6(obj1.method1.call(aliasOfarrObj0 ));
WScript.Echo('a = ' + (a|0));
WScript.Echo('b = ' + (b|0));
WScript.Echo('c = ' + (c|0));
WScript.Echo('d = ' + (d|0));
WScript.Echo('e = ' + (e|0));
WScript.Echo('f = ' + (f|0));
WScript.Echo('g = ' + (g|0));
WScript.Echo('h = ' + (h|0));
WScript.Echo('obj0.prop0 = ' + (obj0.prop0|0));
WScript.Echo('obj0.prop1 = ' + (obj0.prop1|0));
WScript.Echo('obj0.length = ' + (obj0.length|0));
WScript.Echo('obj1.prop0 = ' + (obj1.prop0|0));
WScript.Echo('obj1.prop1 = ' + (obj1.prop1|0));
WScript.Echo('obj1.length = ' + (obj1.length|0));
WScript.Echo('protoObj1.prop0 = ' + (protoObj1.prop0|0));
WScript.Echo('protoObj1.prop1 = ' + (protoObj1.prop1|0));
WScript.Echo('protoObj1.length = ' + (protoObj1.length|0));
WScript.Echo('arrObj0.prop0 = ' + (arrObj0.prop0|0));
WScript.Echo('arrObj0.prop1 = ' + (arrObj0.prop1|0));
WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
WScript.Echo('aliasOfarrObj0.prop0 = ' + (aliasOfarrObj0.prop0|0));
WScript.Echo('aliasOfarrObj0.prop1 = ' + (aliasOfarrObj0.prop1|0));
WScript.Echo('aliasOfarrObj0.length = ' + (aliasOfarrObj0.length|0));
WScript.Echo('protoObj1.prop5 = ' + (protoObj1.prop5|0));
WScript.Echo('uniqobj13.prop0 = ' + (uniqobj13.prop0|0));
WScript.Echo('strvar0 = ' + (strvar0));
WScript.Echo('strvar1 = ' + (strvar1));
WScript.Echo('strvar2 = ' + (strvar2));
WScript.Echo('strvar3 = ' + (strvar3));
WScript.Echo('strvar4 = ' + (strvar4));
WScript.Echo('strvar5 = ' + (strvar5));
WScript.Echo('strvar6 = ' + (strvar6));
WScript.Echo('strvar7 = ' + (strvar7));
WScript.Echo('arrObj0[0] = ' + (arrObj0[0]|0));
WScript.Echo('arrObj0[1] = ' + (arrObj0[1]|0));
WScript.Echo('arrObj0[2] = ' + (arrObj0[2]|0));
WScript.Echo('arrObj0[3] = ' + (arrObj0[3]|0));
WScript.Echo('arrObj0[4] = ' + (arrObj0[4]|0));
WScript.Echo('arrObj0[5] = ' + (arrObj0[5]|0));
WScript.Echo('arrObj0[6] = ' + (arrObj0[6]|0));
WScript.Echo('arrObj0[7] = ' + (arrObj0[7]|0));
WScript.Echo('arrObj0[8] = ' + (arrObj0[8]|0));
WScript.Echo('arrObj0[9] = ' + (arrObj0[9]|0));
WScript.Echo('arrObj0[10] = ' + (arrObj0[10]|0));
WScript.Echo('arrObj0[11] = ' + (arrObj0[11]|0));
WScript.Echo('arrObj0[12] = ' + (arrObj0[12]|0));
WScript.Echo('arrObj0[13] = ' + (arrObj0[13]|0));
WScript.Echo('arrObj0[14] = ' + (arrObj0[14]|0));
WScript.Echo('arrObj0[arrObj0.length-1] = ' + (arrObj0[arrObj0.length-1]|0));
WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
WScript.Echo('ary[0] = ' + (ary[0]|0));
WScript.Echo('ary[1] = ' + (ary[1]|0));
WScript.Echo('ary[2] = ' + (ary[2]|0));
WScript.Echo('ary[3] = ' + (ary[3]|0));
WScript.Echo('ary[4] = ' + (ary[4]|0));
WScript.Echo('ary[5] = ' + (ary[5]|0));
WScript.Echo('ary[6] = ' + (ary[6]|0));
WScript.Echo('ary[7] = ' + (ary[7]|0));
WScript.Echo('ary[8] = ' + (ary[8]|0));
WScript.Echo('ary[9] = ' + (ary[9]|0));
WScript.Echo('ary[10] = ' + (ary[10]|0));
WScript.Echo('ary[11] = ' + (ary[11]|0));
WScript.Echo('ary[12] = ' + (ary[12]|0));
WScript.Echo('ary[13] = ' + (ary[13]|0));
WScript.Echo('ary[14] = ' + (ary[14]|0));
WScript.Echo('ary[ary.length-1] = ' + (ary[ary.length-1]|0));
WScript.Echo('ary.length = ' + (ary.length|0));
for (var i = 0; i < GiantPrintArray.length; i++) {
  WScript.Echo(GiantPrintArray[i]);
  }
;
WScript.Echo('sumOfary = ' +  ary.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_ary = ' +  ary.slice(0, 11));;
WScript.Echo('sumOfIntArr0 = ' +  IntArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_IntArr0 = ' +  IntArr0.slice(0, 11));;
WScript.Echo('sumOfIntArr1 = ' +  IntArr1.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_IntArr1 = ' +  IntArr1.slice(0, 11));;
WScript.Echo('sumOfFloatArr0 = ' +  FloatArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_FloatArr0 = ' +  FloatArr0.slice(0, 11));;
WScript.Echo('sumOfVarArr0 = ' +  VarArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_VarArr0 = ' +  VarArr0.slice(0, 11));;
WScript.Echo('sumOfaliasOfFloatArr0 = ' +  aliasOfFloatArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_aliasOfFloatArr0 = ' +  aliasOfFloatArr0.slice(0, 11));;
`);
WScript.RegisterModuleSource('module3_ea7f0ea1-daf1-4afe-9234-6713ee08ccb0.js', `
var loopInvariant = shouldBailout ? 9 : 7;
var GiantPrintArray = [];
__counter++;;
function makeArrayLength(x) { if(x < 1 || x > 4294967295 || x != x || isNaN(x) || !isFinite(x)) return 100; else return Math.floor(x) & 0xffff; };;
function leaf() { return 100; };
class module3BaseClass { };;
var obj0 = {};
var protoObj0 = {};
var obj1 = {};
var protoObj1 = {};
var arrObj0 = {};
var litObj0 = {prop1: 3.14159265358979};
var litObj1 = {prop0: 0, prop1: 1};
var arrObj0 = {};
var func0 = function(){
  return 501673763;
};
var func1 = function(argMath95,argMath96,...argArr97){
  var strvar9 = strvar3;
  strvar9 = strvar9.substring((strvar9.length)/2,(strvar9.length)/3);
  func0.call(obj0 );
  var re1 = new RegExp("(?=bbabb)", "g");
  return 8.75845953251631E+18;
};
var func2 = function(argMath98,argMath99,argMath100,...argArr101){
  var strvar9 = (strvar3).replace(strvar0, 'p');
  class class12 extends module3BaseClass {
    get func5 (){
      strvar2 = 'p'.concat(((new Array()) instanceof ((typeof Function == 'function' ) ? Function : Object)));
      arrObj0.length= makeArrayLength(f);
      argMath99 =(argMath99 ^ -8.72033334121205E+18);
      GiantPrintArray.push('arrObj0.prop0 = ' + (arrObj0.prop0));
      return f;
    }
    func6 (argMath102 = (new module3BaseClass()),argMath103 = (new module3BaseClass()),argMath104 = ui16[(126) & 255],...argArr105){
      GiantPrintArray.push('argMath102 = ' + (argMath102));
      argMath99 >>='caller';
      WScript.Echo(strvar9 ==('453' in ui16));
      WScript.Echo(strvar9 !==(typeof(argMath100)  == 'boolean') );
      strvar9 = strvar6[0%strvar6.length];
      var strvar10 = strvar3;
      return 1339109296.1;
    }
    func7 (argMath106,argMath107 = ((uic8[(argMath106) & 255] == (new module3BaseClass())) - argMath106),argMath108,...argArr109){
      return 2060450305.1;
    }
    func8 (argMath110,argMath111 = (argArr101.reverse()),argMath112,...argArr113){
      strvar9 = strvar9[2%strvar9.length];
      return 2.69588694979616E+18;
      protoObj1.length= makeArrayLength((611150652.1 ? i32[(('N$yK$' + 'h#O!.3!|'.indexOf('*!±!!' + 'Jg«4$%--'))) & 255] : (argArr113.shift())));
      return e;
    }
  }
  return (true instanceof ((typeof func0 == 'function' ) ? func0 : Object));
};
var func3 = function(argMath114,...argArr115){
  return ([1, 2, 3] instanceof ((typeof Array == 'function' ) ? Array : Object));
};
var func4 = function(){
  var strvar9 = '1!p$w' + '\`*.!Ùé)!';
  class class13 {
    constructor (){
      a = (((d >= arrObj0.prop0)&&(f != protoObj0.prop1)) / ((~ obj1.prop0) == 0 ? 1 : (~ obj1.prop0)));
    }
    func10 (argMath116 = (ary.shift()),argMath117,argMath118 = func3.call(protoObj1 , ary, ary)){
      var fPolyProp = function (o) {
        if (o!==undefined) {
          WScript.Echo(o.prop0 + ' ' + o.prop1 + ' ' + o.prop2);
        }
      };
      fPolyProp(litObj0); 
      fPolyProp(litObj1); 

      return -36898908;
    }
    static func11 (){
(Object.defineProperty(arrObj0, 'prop0', {writable: true, enumerable: false, configurable: true }));
      arrObj0.prop0 = (typeof(strvar2)  != 'boolean') ;
      obj1.length= makeArrayLength((++ obj0.length));
      return obj1.prop1;
    }
  }
  return (ary.shift());
};
obj0.method0 = func1;
obj0.method1 = obj0.method0;
obj1.method0 = func0;
obj1.method1 = func3;
arrObj0.method0 = func4;
arrObj0.method1 = obj0.method0;
var ary = new Array(10);
var i8 = new Int8Array(256);
var i16 = new Int16Array(256);
var i32 = new Int32Array(256);
var ui8 = new Uint8Array(256);
var ui16 = new Uint16Array(256);
var ui32 = new Uint32Array(256);
var f32 = new Float32Array(256);
var f64 = new Float64Array(256);
var uic8 = new Uint8ClampedArray(256);
var IntArr0 = [];
var IntArr1 = new Array(-178235899,-712652185,944431880303876224,65536,7794263011287248896);
var FloatArr0 = new Array(1.17563578796299E+16,-1185908746,-50,-3.28573118411761E+17,255,-81274447.9,-1960249773.9,248167897,251);
var VarArr0 = [strvar0,-180];
var a = 739418103;
var b = 164;
var c = 1325056610.1;
var d = -1378536627.9;
var e = -12911895;
var f = -32;
var g = 935304131.1;
var h = 1588577696;
var strvar0 = 'ª';
var strvar1 = '!' + 'ÉÁ)Á';
var strvar2 = 'ª';
var strvar3 = '¨R'+'îp'+'á#' + '!!';
var strvar4 = '$' + 'b!¢#';
var strvar5 = '!' + '!ð,g';
var strvar6 = 'N$yK$' + 'h#O!.3!|';
var strvar7 = '$' + 'b!¢#';
arrObj0[0] = 181273896;
arrObj0[1] = 408670374;
arrObj0[2] = 1443698706.1;
arrObj0[3] = 149977479.1;
arrObj0[4] = -692472837;
arrObj0[5] = +undefined;
arrObj0[6] = -251785323;
arrObj0[7] = -1783638975.9;
arrObj0[8] = 2.87959361155615E+18;
arrObj0[9] = 127813883;
arrObj0[10] = 3.79246411048011E+18;
arrObj0[11] = 7.35618840863362E+18;
arrObj0[12] = -286258399;
arrObj0[13] = 65537;
arrObj0[14] = -159;
arrObj0[arrObj0.length-1] = 1527059272;
arrObj0.length = makeArrayLength(2.83781467127199E+18);
ary[0] = -757098479;
ary[1] = 65535;
ary[2] = 3.92158125584493E+17;
ary[3] = -2147483648;
ary[4] = 1394941281;
ary[5] = -6.77373561872367E+17;
ary[6] = 162312492;
ary[7] = -3.43730063887663E+18;
ary[8] = 213111824;
ary[9] = 84906731;
ary[10] = -284521977;
ary[11] = 1004820063.1;
ary[12] = 537203022;
ary[13] = 2.17392729819166E+18;
ary[14] = -349730563.9;
ary[ary.length-1] = -Infinity;
ary.length = makeArrayLength(3.61044560317183E+18);
var protoObj0 = Object.create(obj0);
var protoObj1 = Object.create(obj1);
obj0.prop0 = -426009328;
obj0.prop1 = 1024097759.1;
obj0.length = makeArrayLength(-1723634163);
protoObj0.prop0 = -976733935;
protoObj0.prop1 = -1504441050;
protoObj0.length = makeArrayLength(33);
obj1.prop0 = 58;
obj1.prop1 = 45610843.1;
obj1.length = makeArrayLength(-1.67839085100601E+18);
protoObj1.prop0 = -446362123;
protoObj1.prop1 = 1073741823;
protoObj1.length = makeArrayLength(-3.0037802306497E+18);
arrObj0.prop0 = 241;
arrObj0.prop1 = 1445332937;
arrObj0.length = makeArrayLength(6.80561023076584E+18);
IntArr0[0] = -2147483646;
IntArr0[5] = 1152989382.1;
IntArr0[IntArr0.length] = 2;
IntArr0[1] = 4.69135901690752E+18;
IntArr0[3] = -247;
IntArr0[IntArr0.length] = 7.41313024075384E+18;
IntArr1[10] = -1073741824;
IntArr1[IntArr1.length] = -74568381.9;
IntArr1[8] = 278970793;
IntArr1[7] = 410505624.1;
IntArr1[5] = 585048593.1;
IntArr1[6] = -2056018194.9;
VarArr0[VarArr0.length] = 20473628.1;
import { default as module3_localbinding_0, default as module3_localbinding_1, default as module3_localbinding_2 } from 'module0_af60f209-dff4-41e2-8a11-0206e428c198.js';
import { default as module3_localbinding_3, default as module3_localbinding_4 } from 'module2_4c1abe23-d6d6-48ac-9650-608cb0c4a32d.js';
export default (argMath119 = ((arrObj0.prop0 < h)||(arrObj0.prop0 < protoObj0.prop0)),argMath120) => argMath119

export { strvar1 as module3_exportbinding_0, obj0 as module3_exportbinding_1, obj0 as module3_exportbinding_2, g as module3_exportbinding_3 };
export {  } from 'module1_afa96955-9820-4822-9b34-005434e9a93e.js';
obj1.prop1 = (c != protoObj1.prop1);
export { };
WScript.Echo('a = ' + (a|0));
WScript.Echo('b = ' + (b|0));
WScript.Echo('c = ' + (c|0));
WScript.Echo('d = ' + (d|0));
WScript.Echo('e = ' + (e|0));
WScript.Echo('f = ' + (f|0));
WScript.Echo('g = ' + (g|0));
WScript.Echo('h = ' + (h|0));
WScript.Echo('module3_localbinding_0 = ' + (module3_localbinding_0|0));
WScript.Echo('module3_localbinding_1 = ' + (module3_localbinding_1|0));
WScript.Echo('module3_localbinding_2 = ' + (module3_localbinding_2|0));
WScript.Echo('module3_localbinding_3 = ' + (module3_localbinding_3|0));
WScript.Echo('module3_localbinding_4 = ' + (module3_localbinding_4|0));
WScript.Echo('obj0.prop0 = ' + (obj0.prop0|0));
WScript.Echo('obj0.prop1 = ' + (obj0.prop1|0));
WScript.Echo('obj0.length = ' + (obj0.length|0));
WScript.Echo('protoObj0.prop0 = ' + (protoObj0.prop0|0));
WScript.Echo('protoObj0.prop1 = ' + (protoObj0.prop1|0));
WScript.Echo('protoObj0.length = ' + (protoObj0.length|0));
WScript.Echo('obj1.prop0 = ' + (obj1.prop0|0));
WScript.Echo('obj1.prop1 = ' + (obj1.prop1|0));
WScript.Echo('obj1.length = ' + (obj1.length|0));
WScript.Echo('protoObj1.prop0 = ' + (protoObj1.prop0|0));
WScript.Echo('protoObj1.prop1 = ' + (protoObj1.prop1|0));
WScript.Echo('protoObj1.length = ' + (protoObj1.length|0));
WScript.Echo('arrObj0.prop0 = ' + (arrObj0.prop0|0));
WScript.Echo('arrObj0.prop1 = ' + (arrObj0.prop1|0));
WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
WScript.Echo('strvar0 = ' + (strvar0));
WScript.Echo('strvar1 = ' + (strvar1));
WScript.Echo('strvar2 = ' + (strvar2));
WScript.Echo('strvar3 = ' + (strvar3));
WScript.Echo('strvar4 = ' + (strvar4));
WScript.Echo('strvar5 = ' + (strvar5));
WScript.Echo('strvar6 = ' + (strvar6));
WScript.Echo('strvar7 = ' + (strvar7));
WScript.Echo('arrObj0[0] = ' + (arrObj0[0]|0));
WScript.Echo('arrObj0[1] = ' + (arrObj0[1]|0));
WScript.Echo('arrObj0[2] = ' + (arrObj0[2]|0));
WScript.Echo('arrObj0[3] = ' + (arrObj0[3]|0));
WScript.Echo('arrObj0[4] = ' + (arrObj0[4]|0));
WScript.Echo('arrObj0[5] = ' + (arrObj0[5]|0));
WScript.Echo('arrObj0[6] = ' + (arrObj0[6]|0));
WScript.Echo('arrObj0[7] = ' + (arrObj0[7]|0));
WScript.Echo('arrObj0[8] = ' + (arrObj0[8]|0));
WScript.Echo('arrObj0[9] = ' + (arrObj0[9]|0));
WScript.Echo('arrObj0[10] = ' + (arrObj0[10]|0));
WScript.Echo('arrObj0[11] = ' + (arrObj0[11]|0));
WScript.Echo('arrObj0[12] = ' + (arrObj0[12]|0));
WScript.Echo('arrObj0[13] = ' + (arrObj0[13]|0));
WScript.Echo('arrObj0[14] = ' + (arrObj0[14]|0));
WScript.Echo('arrObj0[arrObj0.length-1] = ' + (arrObj0[arrObj0.length-1]|0));
WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
WScript.Echo('ary[0] = ' + (ary[0]|0));
WScript.Echo('ary[1] = ' + (ary[1]|0));
WScript.Echo('ary[2] = ' + (ary[2]|0));
WScript.Echo('ary[3] = ' + (ary[3]|0));
WScript.Echo('ary[4] = ' + (ary[4]|0));
WScript.Echo('ary[5] = ' + (ary[5]|0));
WScript.Echo('ary[6] = ' + (ary[6]|0));
WScript.Echo('ary[7] = ' + (ary[7]|0));
WScript.Echo('ary[8] = ' + (ary[8]|0));
WScript.Echo('ary[9] = ' + (ary[9]|0));
WScript.Echo('ary[10] = ' + (ary[10]|0));
WScript.Echo('ary[11] = ' + (ary[11]|0));
WScript.Echo('ary[12] = ' + (ary[12]|0));
WScript.Echo('ary[13] = ' + (ary[13]|0));
WScript.Echo('ary[14] = ' + (ary[14]|0));
WScript.Echo('ary[ary.length-1] = ' + (ary[ary.length-1]|0));
WScript.Echo('ary.length = ' + (ary.length|0));
for (var i = 0; i < GiantPrintArray.length; i++) {
  WScript.Echo(GiantPrintArray[i]);
  }
;
WScript.Echo('sumOfary = ' +  ary.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_ary = ' +  ary.slice(0, 11));;
WScript.Echo('sumOfIntArr0 = ' +  IntArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_IntArr0 = ' +  IntArr0.slice(0, 11));;
WScript.Echo('sumOfIntArr1 = ' +  IntArr1.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_IntArr1 = ' +  IntArr1.slice(0, 11));;
WScript.Echo('sumOfFloatArr0 = ' +  FloatArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_FloatArr0 = ' +  FloatArr0.slice(0, 11));;
WScript.Echo('sumOfVarArr0 = ' +  VarArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_VarArr0 = ' +  VarArr0.slice(0, 11));;
`);
WScript.RegisterModuleSource('module4_e548fb48-aed8-443c-8b33-45dee06705f5.js', `
var loopInvariant = shouldBailout ? 8 : 1;
var GiantPrintArray = [];
__counter++;;
function makeArrayLength(x) { if(x < 1 || x > 4294967295 || x != x || isNaN(x) || !isFinite(x)) return 100; else return Math.floor(x) & 0xffff; };;
function leaf() { return 100; };
class module4BaseClass { };;
var obj0 = {};
var protoObj0 = {};
var obj1 = {};
var protoObj1 = {};
var arrObj0 = {};
var litObj0 = {prop1: 3.14159265358979};
var litObj1 = {prop0: 0, prop1: 1};
var arrObj0 = {};
var func0 = function(){
  strvar6 = strvar2[6%strvar2.length];
  return ((leaf.call(obj1 ), leaf.call(protoObj0 ), ((ary.reverse()) ? 'caller' : (typeof(strvar0)  == 'boolean') ), (new module4BaseClass()), (typeof(obj1.prop0)  != 'boolean') ) * (h != h) - g);
};
var func1 = function(){
  litObj0.prop1 = (('C7%÷Ì' + '!¯ÝQ½!Cm'.indexOf('s')) >>> protoObj0.prop1);
  return 'caller';
};
var func2 = function(argMath121,argMath122,argMath123){
  argMath122 &=(new func1()).prop0 ;
  class class14 extends module4BaseClass {
    constructor (argMath124 = argMath121){
      super();
      var uniqobj14 = protoObj0;
(Object.defineProperty(litObj0, 'prop2', {writable: true, enumerable: false, configurable: true }));
      litObj0.prop2 = (typeof (argMath122 = (('ÛW(t.lpfu1³A)+@'.indexOf(',')) instanceof ((typeof String == 'function' ) ? String : Object))));
      litObj1 = litObj1;
      WScript.Echo(strvar6 >(uniqobj14.prop0 instanceof ((typeof Number == 'function' ) ? Number : Object)));
    }
    func6 (argMath125 = (typeof(obj1.prop0)  == null) ,argMath126,argMath127,argMath128){
      if(shouldBailout){
        return  'somestring'
      }
      argMath126 = arguments[((((((new RangeError()) instanceof ((typeof Error == 'function' ) ? Error : Object)) * (argMath127 + argMath127)) >= 0 ? (((new RangeError()) instanceof ((typeof Error == 'function' ) ? Error : Object)) * (argMath127 + argMath127)) : 0)) & 0XF)];
      return argMath128;
    }
    func7 (argMath129,argMath130){
      b =(e * 1772246304);
      strvar2 = strvar1[5%strvar1.length];
      return -9.38228475621423E+17;
    }
    func8 (){
      litObj1 = litObj0;
      return -9.03828705924849E+18;
      WScript.Echo(strvar4 >(argMath121 &= 460251107));
      var w = (((f32[2147483647], ((c === c) === 3), f32[2147483647], (argMath123 * arguments[(18)] + obj1.prop1)) * (strvar4 - ((new Error('abc')) instanceof ((typeof func0 == 'function' ) ? func0 : Object)))) * (((argMath123 * arguments[(18)] + obj1.prop1) * (f32[2147483647], ((c === c) === 3), f32[2147483647], (argMath123 * arguments[(18)] + obj1.prop1)) + ((('Ôv'+'^#'+'ÉU' + '!)'.indexOf((strvar0 + ('!'.indexOf('%' + 'ô,!¶'))))) ? -2 : (ary.slice(4,11))) * (obj0.prop1 ? (0 ? -525053071 : (typeof(strvar2)  == 'string') ) : obj1.prop0))) - func1.call(litObj0 )));
      argMath121 = (typeof(b)  == 'number') ;
      return w;
    }
    static set func9 (argMath131){
      return arrObj0.prop1;
    }
    static func10 (argMath132 = (('ÞÚE$E' + 'b(V$$!ôß'.indexOf(('@' + '%#!$').replace(/a/g, '!.'+'!!'+'Ü|' + '%Á'))) ? parseInt("-6783AE76", 15) : (typeof(strvar1)  != 'undefined') ),argMath133,argMath134){
      argMath133 =(argMath132 != obj1.prop0);
      return -813300597;
    }
  }
  return (new module4BaseClass());
};
var func3 = function(){
  class class15 extends module4BaseClass {
    constructor (...argArr135){
      super();
      var fPolyProp = function (o) {
        if (o!==undefined) {
          WScript.Echo(o.prop0 + ' ' + o.prop1 + ' ' + o.prop2);
        }
      };
      fPolyProp(litObj0); 
      fPolyProp(litObj1); 

      protoObj0.prop0 +=ary[(((argArr135 >= 0 ? argArr135 : 0)) & 0XF)];
      WScript.Echo(strvar4 ===(strvar0 + (76710602.1 ? -354742655 : -745579168.9)));
      WScript.Echo(strvar3 >(-131535588 != -2147483646));
      if(shouldBailout){
        return  'somestring'
      }
      strvar3 = strvar0[6%strvar0.length];
    }
    func12 (argMath136,argMath137,argMath138 = ((new Array()) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)),...argArr139){
      strvar7 = ('s' + func0.call(obj0 )) + (protoObj0.prop0 && -7.74318286692901E+18);
      protoObj0 = protoObj0;
(Object.defineProperty(protoObj1, 'prop1', {writable: true, enumerable: false, configurable: true }));
      protoObj1.prop1 = (typeof(arrObj0.prop1)  == null) ;
      strvar4 = 'ÛW(t.lpfu1³A)+@'.concat(-1103248723);
      f = (argMath138 >>>= (protoObj1.length = (typeof(arrObj0.prop1)  == null) ));
      return -2070707709.9;
    }
    func13 (){
      strvar7 = (strvar0.concat('caller')).replace('^R'+'bB'+'·o' + 'u#', ('4' + 'a$a!' + arguments[(((-1205325163 >= 0 ? -1205325163 : 0)) & 0XF)]).concat(((a > protoObj0.prop0)&&(a === c)))) + (typeof(f)  == 'object') ;
      strvar3 = strvar0 + (87 * protoObj1.prop0 + obj0.prop0);
      return -512038945;
    }
    set func14 (argMath140 = (protoObj0.length &= (typeof(arguments[(14)])  == 'string') )){
      var strvar9 = strvar5;
      if(shouldBailout){
        return  'somestring'
      }
      WScript.Echo(strvar2 <(++ e));
      obj0.prop0 %=(i16[(74) & 255] * ('caller' - (new module4BaseClass())));
      strvar9 = ('Ôv'+'^#'+'ÉU' + '!)' + (-- h)) + (d ^= 127);
      return protoObj1.prop1;
    }
    func15 (){
      arguments[(3)] = (-1 < arrObj0.prop0);
      litObj1 = litObj1;
      return 3.64737574678373E+18;
    }
    static func16 (argMath141,argMath142,argMath143,...argArr144){
      if(shouldBailout){
        return  'somestring'
      }
(Object.defineProperty(litObj0, 'prop3', {writable: true, enumerable: false, configurable: true }));
      litObj0.prop3 = (argArr144.shift());
      return -3.19857079407113E+18;
    }
    static func17 (){
      eval("");
      return -2123851943;
    }
    static func18 (...argArr145){
      WScript.Echo(strvar3 !=='caller');
      var strvar9 = '%' + 'ô,!¶';
      strvar4 = strvar9 + ((protoObj1.prop0 === g)||(arrObj0.prop0 !== a));
      c =(h % -1802546355.9);
      if(shouldBailout){
        return  'somestring'
      }
      obj0.prop4 = (typeof((h % -1802546355.9))  != 'boolean') ;
      return 2.84444888744292E+18;
    }
    static func19 (){
      strvar3 = strvar5[0%strvar5.length];
      var uniqobj15 = protoObj0;
      strvar6 = strvar5[4%strvar5.length];
      return uniqobj15.prop1;
    }
  }
  func1.call(obj1 );
  return ((protoObj1.prop0 == protoObj1.prop1)||(protoObj0.prop1 >= protoObj1.prop0));
};
var func4 = function(argMath146){
  return (obj1.length-- );
};
obj0.method0 = func1;
obj0.method1 = func1;
obj1.method0 = obj0.method1;
obj1.method1 = func3;
arrObj0.method0 = func0;
arrObj0.method1 = obj1.method1;
var ary = new Array(10);
var i8 = new Int8Array(256);
var i16 = new Int16Array(256);
var i32 = new Int32Array(256);
var ui8 = new Uint8Array(256);
var ui16 = new Uint16Array(256);
var ui32 = new Uint32Array(256);
var f32 = new Float32Array(256);
var f64 = new Float64Array(256);
var uic8 = new Uint8ClampedArray(256);
var IntArr0 = new Array(1,-464465545287772416,1548002816,4125977493990329856,-378242901,-929061088,-61,-66);
var IntArr1 = new Array(610351421,-2147483648,-2240049058324462848,4283368681238733824,-243);
var FloatArr0 = [];
var VarArr0 = [];
var a = -977207319;
var b = -180;
var c = -150;
var d = 65537;
var e = -416748135;
var f = -617330253;
var g = -8.87961186168574E+18;
var h = 909767047;
var strvar0 = '@' + '%#!$';
var strvar1 = 'ÛW(t.lpfu1³A)+@';
var strvar2 = 's';
var strvar3 = 'b2#S,%ô,Ì,b=²$+';
var strvar4 = 's';
var strvar5 = '#,$òq' + '<¥r!3+#-';
var strvar6 = 'Ö';
var strvar7 = '¼' + '! #Q';
arrObj0[0] = 51;
arrObj0[1] = -1718987986.9;
arrObj0[2] = 2147483650;
arrObj0[3] = 920628107;
arrObj0[4] = 438227936;
arrObj0[5] = 16648723;
arrObj0[6] = -2105549134.9;
arrObj0[7] = -1280454579;
arrObj0[8] = 1;
arrObj0[9] = 1073741823;
arrObj0[10] = -131;
arrObj0[11] = 1057683180.1;
arrObj0[12] = 228;
arrObj0[13] = 1778225674;
arrObj0[14] = 1227863859.1;
arrObj0[arrObj0.length-1] = 385591801.1;
arrObj0.length = makeArrayLength(-616526588);
ary[0] = 3;
ary[1] = 834249843;
ary[2] = -7.82428706656811E+18;
ary[3] = 1;
ary[4] = -525449277.9;
ary[5] = 3.26412835733346E+18;
ary[6] = -660757122;
ary[7] = -2007082397.9;
ary[8] = -45552146;
ary[9] = 26;
ary[10] = -320864180;
ary[11] = 1.83550311878076E+18;
ary[12] = 65536;
ary[13] = 1306372002;
ary[14] = 641214054;
ary[ary.length-1] = 70;
ary.length = makeArrayLength(1291846221.1);
var protoObj0 = Object.create(obj0);
var protoObj1 = Object.create(obj1);
obj0.prop0 = -159;
obj0.prop1 = 224;
obj0.length = makeArrayLength(-211);
protoObj0.prop0 = 1;
protoObj0.prop1 = 816295006;
protoObj0.length = makeArrayLength(-3.38669264902448E+18);
obj1.prop0 = 41;
obj1.prop1 = 3;
obj1.length = makeArrayLength(-65305269);
protoObj1.prop0 = 428519531;
protoObj1.prop1 = 65535;
protoObj1.length = makeArrayLength(173);
arrObj0.prop0 = 3;
arrObj0.prop1 = 1031908327.1;
arrObj0.length = makeArrayLength(1216859869);
obj0.prop4 = -2.03854884556037E+18;
IntArr0[9] = 201197980;
IntArr0[8] = -1374861330.9;
IntArr1[7] = -238213384;
IntArr1[5] = -7.16372202344362E+18;
IntArr1[6] = -1903197904;
IntArr1[8] = 198;
IntArr1[9] = -246;
IntArr1[10] = -2147483648;
IntArr1[12] = 2147483650;
IntArr1[IntArr1.length] = 352415869;
FloatArr0[0] = 138089796;
FloatArr0[1] = -7.8190659623414E+18;
FloatArr0[2] = -1.50223246074391E+18;
VarArr0[1] = -240;
VarArr0[0] = 687891451.1;
VarArr0[VarArr0.length] = -1463024259.9;
VarArr0[3] = 10;
VarArr0[2] = 1.83324386450987E+18;
VarArr0[4] = -9;
strvar2 = strvar4[0%strvar4.length];
obj0.prop1 =(-1530317899.9 % 1602497539);
import {  } from 'module3_ea7f0ea1-daf1-4afe-9234-6713ee08ccb0.js';
import { default as module4_localbinding_0, module3_exportbinding_2 as module4_localbinding_1 } from 'module3_ea7f0ea1-daf1-4afe-9234-6713ee08ccb0.js';
class class16 {
  constructor (){
    litObj1.prop0=((obj0.prop1 <= a) ? arguments[(12)] : ((('#)!!$' + 'pL,}$#!('.concat(-1778905159.9) + (b = (arrObj0.prop0 != obj0.prop0))) + (f = (new module4BaseClass())))).replace(/a/g, '#,$òq' + '<¥r!3+#-'));
    strvar1 = strvar5.concat(((typeof(protoObj1.prop1)  == null)  * ('caller' + ((new Error('abc')) instanceof ((typeof EvalError == 'function' ) ? EvalError : Object)))));
    var __loopvar2 = loopInvariant,__loopSecondaryVar2_0 = loopInvariant - 3,__loopSecondaryVar2_1 = loopInvariant;
    LABEL0: 
    LABEL1: 
    for (var _strvar1 in ary) {
      if(typeof _strvar1 === 'string' && _strvar1.indexOf('method') != -1) continue;
      __loopvar2++;
      if (__loopvar2 >= loopInvariant + 3) break;
      __loopSecondaryVar2_0++;
      __loopSecondaryVar2_1 += 4;
      ary[_strvar1] = func0.call(arrObj0 );
    }
    var __loopvar2 = loopInvariant,__loopSecondaryVar2_0 = loopInvariant - 3,__loopSecondaryVar2_1 = loopInvariant;
    LABEL0: 
    LABEL1: 
    for(; b < ((strvar6 + ('litObj1.prop0' in arrObj0))); b++) {
      __loopSecondaryVar2_0++;
      __loopSecondaryVar2_1 += 2;
      if (__loopvar2 == loopInvariant - 9) break;
      __loopvar2 -= 3;
      var uniqobj16 = [obj0, obj1, protoObj0];
      uniqobj16[__counter%uniqobj16.length].method1();
      //Snippet From Var   
      function __test0(__arg1) {
        return -(__arg1 + 1);
      }
      GiantPrintArray.push(__test0(1));
      GiantPrintArray.push(__test0(1.1));
      
      strvar1 = strvar3[6%strvar3.length];
      if(shouldBailout){
        return  'somestring'
      }
    }
    //Snippet Google Maps from Old Email 
    //Bug#1133163 
    function lga()
    {
      obj0.length = 0;
      obj1.length= 0;
      module4_localbinding_0= 0;
      if (obj0.length) 
      {
        obj1.length += module4_localbinding_0 ? 2 : 1;
      }
    }
    lga();
    
    
    function func21 () {
      this.prop0 = ((obj0.prop0 %= strvar5) >>> ((obj1.prop1 %= (c ? 302915824 : 221516637)) instanceof ((typeof String == 'function' ) ? String : Object)));
      this.prop2 = 'caller';
    }
    var uniqobj17 = new func21();
    if (shouldBailout) {
      (shouldBailout ? (uniqobj17.prop2 = { valueOf: function() { WScript.Echo('uniqobj17.prop2 valueOf'); return 3; } }, (strvar6).replace(strvar2, (strvar1).replace(strvar7, 'b2#S,%ô,Ì,b=²$+')).concat((protoObj0.prop0 /= arguments[(17)]))) : (strvar6).replace(strvar2, (strvar1).replace(strvar7, 'b2#S,%ô,Ì,b=²$+')).concat((protoObj0.prop0 /= arguments[(17)])));
    }
  }
  func22 (argMath147,argMath148 = (typeof('#,$òq' + '<¥r!3+#-')  != 'string') ,argMath149 = (obj1.prop0 = (protoObj1.prop0 == g)),argMath150){
    class class17 extends module4BaseClass {
      constructor (argMath151){
        super();
        strvar7 = strvar4[6%strvar4.length];
        strvar7 = strvar1 + (obj0.prop1++ );
        var strvar9 = strvar2;
        obj0.prop4 |='caller';
        strvar1 = strvar6[0%strvar6.length];
      }
    }
    return Math.min(IntArr0[(((argMath150 >= 0 ? argMath150 : 0)) & 0XF)], ((typeof(obj0.prop4)  != 'number')  ? (argMath149 ? Object.create(arrObj0) : f) : ((typeof(argMath150)  == 'number')  - argMath150)));
  }
  static func24 (argMath152,...argArr153){
    class class18 extends module4BaseClass {
      constructor (){
        super();
(Object.defineProperty(protoObj1, 'prop0', {writable: true, enumerable: false, configurable: true }));
        protoObj1.prop0 = obj0.prop4;
        if(shouldBailout){
          return  'somestring'
        }
        return g;
      }
      func26 (){
        WScript.Echo(strvar4 >('4' + 'a$a!'.indexOf('Ôv'+'^#'+'ÉU' + '!)')));
        var strvar9 = ((strvar4).replace(/a/g, ('%' + 'ô,!¶' + (protoObj1.prop1 <<= strvar4)))).replace('%' + 'ô,!¶', strvar7);
        var x = (((strvar4).replace(/a/g, ('%' + 'ô,!¶' + (protoObj1.prop1 <<= strvar4)))).replace('%' + 'ô,!¶', strvar7), obj1.method0.call(arrObj0 ), 342629150, 'caller');
        return argMath152;
      }
      func27 (){
        IntArr1[(((protoObj0.prop0 >= 0 ? protoObj0.prop0 : 0)) & 0XF)] = (-- arrObj0.length);
        var strvar9 = (strvar5 + uic8[(190) & 255]);
        return arrObj0.prop1;
      }
      static func28 (){
        GiantPrintArray.push('e = ' + (e));
        strvar0 = strvar3[2%strvar3.length];
        WScript.Echo('argMath152 = ' + (argMath152));
        argMath152 = IntArr0[(((arrObj0[(0)] >= 0 ? arrObj0[(0)] : 0)) & 0XF)];
        WScript.Echo(strvar0 !=(new module4BaseClass()));
        strvar6 = 'Ö'.concat(strvar0);
        return argMath152;
      }
    }
    e = (-- h);
    return -1073741824;
  }
  static func29 (argMath154 = (typeof(h)  == 'number') ){
    // apply8.ecs
    var v8 = {
      init : function() {
        return function bar() {
          var reResult1=/(ab)|abab|\w\b\d|babbb/myu.test('-RÜ#$' + '!«((*nf(');
          this.method0.apply(protoObj0, arguments);
        }
      }
    };
    class class19 extends module4BaseClass {
      constructor (){
        super();
        GiantPrintArray.push('argMath154 = ' + (argMath154));
        strvar7 = strvar1[0%strvar1.length];
        strvar1 = '-RÜ#$' + '!«((*nf(' + func1.call(arrObj0 );
        protoObj1.prop1 *=(f64[1275898430.9] * (obj1.prop0 - protoObj1.method0.call(litObj1 )));
        strvar2 = strvar5.concat((typeof(arrObj0.prop1)  == null) ) + -787019827;
      }
      func31 (argMath155,argMath156 = (((argMath155 ? argMath155 : (arrObj0.method0.call(obj0 ) ? (ui32[1135911276.1] * (ary[(((4.74300293256183E+18 >= 0 ? 4.74300293256183E+18 : 0)) & 0XF)] - (argMath155 < (3 ? 2.33017958358912E+18 : argMath155)))) : (((a <= argMath155)&&(e === g)) ? ((-1841911917 instanceof ((typeof EvalError == 'function' ) ? EvalError : Object)) * (-429104081 ? -71300263 : -431799578.9) + ui16[(arrObj0.prop1) & 255]) : ((argMath155, 1718334364) + arguments[(11)])))) ^ func1.call(protoObj0 )) >> (new module4BaseClass())),argMath157 = (func4.call(obj1 , (typeof(h)  == 'number') ) ? (typeof(h)  == 'number')  : (typeof(protoObj0.prop1)  == 'string') ),...argArr158){
        h =(typeof(strvar4)  != 'object') ;
        var fPolyProp = function (o) {
          if (o!==undefined) {
            WScript.Echo(o.prop0 + ' ' + o.prop1 + ' ' + o.prop2);
          }
        };
        fPolyProp(litObj0); 
        fPolyProp(litObj1); 
    
        g <<=arrObj0.method1.call(protoObj0 );
        var fPolyProp = function (o) {
          if (o!==undefined) {
            WScript.Echo(o.prop0 + ' ' + o.prop1 + ' ' + o.prop2);
          }
        };
        fPolyProp(litObj0); 
        fPolyProp(litObj1); 
    
        return -669052040;
      }
      func32 (){
        g = (new module4BaseClass());
        return 947489968;
      }
      func33 (argMath159,argMath160,argMath161){
        return -3.25524794745221E+17;
      }
      static func34 (){
        WScript.Echo(strvar3 <=(new module4BaseClass()));
        return 3;
      }
      static func35 (argMath162 = ((((f <<= module4_localbinding_1) - (! ((-- argMath154) * ((~ (argMath154 * (g - 1200144361.1))) + ([1, 2, 3] instanceof ((typeof Object == 'function' ) ? Object : Object)))))) * (ui32[(217) & 255] ? 7.27098054718798E+18 : b) + IntArr1[(7)]) != (typeof(('b2#S,%ô,Ì,b=²$+'.concat(obj1.method0.call(obj0 )) + ((module4_localbinding_0 < h) * ((((new module4BaseClass()) && (argMath154 === arrObj0.prop1)) <= (arguments[(((obj0.prop1 >= 0 ? obj0.prop1 : 0)) & 0XF)] * ((c ? argMath154 : arrObj0.prop1) - (obj0.prop1 != argMath154)))) + (arrObj0.method1.call(obj1 ) ? (argMath154 = -85953737) : (argMath154 > g))))))  == 'number') ),argMath163,argMath164 = arrObj0[((((protoObj0.length >>>= (((argMath162++ ) % uic8[(argMath163) & 255]) ? arguments[(((((new RegExp('xyz')) instanceof ((typeof Array == 'function' ) ? Array : Object)) >= 0 ? ((new RegExp('xyz')) instanceof ((typeof Array == 'function' ) ? Array : Object)) : 0)) & 0XF)] : (obj0.length >>= module4_localbinding_0))) >= 0 ? (protoObj0.length >>>= (((argMath162++ ) % uic8[(argMath163) & 255]) ? arguments[(((((new RegExp('xyz')) instanceof ((typeof Array == 'function' ) ? Array : Object)) >= 0 ? ((new RegExp('xyz')) instanceof ((typeof Array == 'function' ) ? Array : Object)) : 0)) & 0XF)] : (obj0.length >>= module4_localbinding_0))) : 0)) & 0XF)],argMath165 = (VarArr0.reverse())){
        WScript.Echo(strvar7 >(arrObj0.prop1 *= -6.46468860839344E+18));
        argMath154 = a;
        var uniqobj18 = {nd0: {lf0: {prop0: 872114137, prop1: -4.97372977408648E+18, length: 4294967297 , method0: arrObj0.method0, method1: obj1.method1}}, nd1: {lf0: {prop0: -184812135, prop1: 868612755.1, length: -1954773487 , method0: func3, method1: func1}, lf1: {prop0: -175384455.9, prop1: -2147483647, length: 15 , method0: func1, method1: func0}, nd2: {lf0: {prop0: -312889744, prop1: -703258884, length: 194 , method0: func2, method1: obj0.method1}}, lf3: {prop0: 866632686, prop1: +null, length: -710335727 , method0: func2, method1: obj1.method1}}, lf2: {prop0: -1951626048, prop1: 0, length: 5.30723648174796E+18 , method0: arrObj0.method0, method1: func3}};
        uniqobj18.nd1.lf1.prop0 =(uniqobj18.nd0.lf0.prop1 > argMath165);
        return uniqobj18.nd1.lf0.prop1;
      }
      static set func36 (argMath166){
        strvar6 = strvar7.concat((arrObj0.prop1 << f));
        obj1.length= makeArrayLength(-1717031454);
        return -482694793.9;
      }
      static func37 (argMath167 = arguments[(((((protoObj1.prop0 <= h)||(f < protoObj1.prop0)) >= 0 ? ((protoObj1.prop0 <= h)||(f < protoObj1.prop0)) : 0)) & 0XF)],argMath168,...argArr169){
        var uniqobj19 = {13: 284433664, 95: arrObj0[(13)], prop0: ((- (argMath168 = (d < module4_localbinding_1))) <= obj0.method0.call(arrObj0 )), prop1: ('!.'+'F.'+'g!' + '$*').replace('!.'+'F.'+'g!' + '$*', (('4' + 'a$a!' + (- (argMath168 = (d < module4_localbinding_1))))).replace(strvar3, strvar5)), prop2: (new module4BaseClass()), prop3: {prop0: (typeof(argMath167)  == 'object') , prop1: (typeof(argMath154)  == null) , prop2: (protoObj0.prop0 * (argMath168 - i8[(138) & 255])), prop3: i8.length, prop4: (arguments[(3)] >= 'caller'), prop5: ((protoObj0.method0.call(obj0 ) ? i32[(((argMath154 >= argMath168)||(argMath167 >= arrObj0.prop1))) & 255] : ((new module4BaseClass()) ? -1555676888.9 : a)) != (new module4BaseClass()))}, prop4: (typeof(h)  == 'number') };
        var strvar9 = '^R'+'bB'+'·o' + 'u#';
        uniqobj19.prop6 = (new module4BaseClass());
        strvar7 = strvar5[3%strvar5.length];
        WScript.Echo(strvar9 <=('¼' + '! #Q'.indexOf(strvar4)));
        argMath167 = (obj1.method1.call(protoObj1 ) instanceof ((typeof String == 'function' ) ? String : Object));
        return 64;
      }
    }
    class19.method1 = v8.init();
    class19.method1.prototype = {
        method0 : function(){
            return obj0.length;
        },
        method1 : function(){
            class class20 extends module4BaseClass {
              constructor (argMath170){
                super();
                WScript.Echo(strvar7 >=(180544562.1 ? module4_localbinding_0 : class19.func36));
                litObj0.prop4 = (IntArr1.unshift((new module4BaseClass()), (ui32[((obj1.method0.call(litObj0 ) ? argMath170 : (-2010529454 * ('Æ' + 'o%!('.indexOf(strvar7)) - ((module4_localbinding_1 < protoObj0.prop0)||(module4_localbinding_1 > class19.func36))))) & 255] + (typeof(protoObj0.prop0)  != 'boolean') ), (new class19()), i32[(140) & 255]));
                var strvar9 = 's'.concat('caller');
              }
              func39 (argMath171,argMath172,argMath173,...argArr174){
                var strvar9 = '!.'+'!!'+'Ü|' + '%Á'.concat(((++ d), (i32[((argMath173 <<= arguments[(18)])) & 255] != arrObj0.method1.call(arrObj0 )), {prop7: (argMath172 = ((new RegExp('xyz')) instanceof ((typeof Number == 'function' ) ? Number : Object))), prop6: (-- d), prop5: ui16[(b) & 255], prop3: func1.call(litObj0 ), prop2: (a % (-1858362896.9 | (arrObj0.method1() ? FloatArr0[(((-8.7520506146415E+18 >= 0 ? -8.7520506146415E+18 : 0)) & 0XF)] : f64[(obj1.prop0) & 255]))), prop0: func4.call(protoObj1 , ('caller' ? ((70376522 ^ obj1.prop1) * (896917153.1 == -2) - (new module4BaseClass())) : (module4_localbinding_0++ ))), 83: f64[(obj1.prop0) & 255], 0: ((arrObj0.prop1 >= argMath172)&&(arrObj0.prop1 != f))}, ((protoObj0.length /= ui16[(b) & 255]) ? arguments[(((73 >= 0 ? 73 : 0)) & 0XF)] : (typeof((('^R'+'bB'+'·o' + 'u#' + (new class19())) + arrObj0[(10)]))  != 'undefined') )));
                WScript.Echo(strvar5 !=(- obj0.prop0));
                strvar6 = (strvar5.concat((new protoObj1.method0()).prop1 )).replace(/a/g, ('-RÜ#$' + '!«((*nf(').replace('-RÜ#$' + '!«((*nf(', '#)!!$' + 'pL,}$#!(')) + (module4_localbinding_0 - -240);
                strvar9 = strvar5.concat((-1223569068.9 ? argMath172 : -27));
                return 687699126;
              }
              get func40 (){
                var strvar9 = (('#)!!$' + 'pL,}$#!('.concat((new class19())) + ((-1286165989.9 << class19.func36) * ((ary.shift()) >= (class19.func36 !== obj1.prop1)) + (typeof(class19.func36)  == 'string') ))).replace(('#)!!$' + 'pL,}$#!('.concat((new class19())) + ((-1286165989.9 << class19.func36) * ((ary.shift()) >= (class19.func36 !== obj1.prop1)) + (typeof(class19.func36)  == 'string') )), ('%' + 'ô,!¶' + (class19.func36 > module4_localbinding_1)).concat(class19.func36).concat((ary.shift())));
                strvar0 = strvar3[6%strvar3.length];
                obj0 = obj0;
                obj1 = obj1;
                var fPolyProp = function (o) {
                  if (o!==undefined) {
                    WScript.Echo(o.prop0 + ' ' + o.prop1 + ' ' + o.prop2);
                  }
                };
                fPolyProp(litObj0); 
                fPolyProp(litObj1); 
            
                return -1923535727;
              }
              func41 (argMath175 = (typeof(obj0.prop4)  == 'object') ,argMath176,...argArr177){
                protoObj0 = arrObj0;
                protoObj0.prop1 /=(~ (strvar0).replace(strvar0, (',' + 'caller')));
                var strvar9 = strvar4;
                protoObj1 = arrObj0;
                var id26 = ((~ (strvar0).replace(strvar0, (',' + 'caller'))) > h);
            (Object.defineProperty(arrObj0, 'prop0', {writable: true, enumerable: false, configurable: true }));
                arrObj0.prop0 = obj0.method1.call(class19 );
                return a;
              }
            }
            return a;
        }
    }
    litObj1 = new class19.method1((((class19.func36 == class19.func36)&&(h === class19.func36)) & (((class19.func36 == class19.func36)&&(h === class19.func36)) ? ((class19.func36 == class19.func36)&&(h === class19.func36)) : VarArr0[((((1.05760487188363E+18 % -2147483649) >= 0 ? (1.05760487188363E+18 % -2147483649) : 0)) & 0XF)])));
    
    return (obj0.prop4 < module4_localbinding_0);
  }
  static func42 (argMath178 = uic8[(243) & 255]){
    strvar2 = strvar0 + Math.tan((typeof(module4_localbinding_0)  == 'boolean') );
    strvar2 = strvar4.concat(i8[(arguments[((((new module4BaseClass()) >= 0 ? (new module4BaseClass()) : 0)) & 0XF)]) & 255]);
    class class21 extends module4BaseClass {
      constructor (argMath179,argMath180,argMath181,...argArr182){
        super();
        WScript.Echo(strvar7 ===(+ protoObj1.prop1));
        strvar7 = strvar0[0%strvar0.length];
        strvar7 = strvar4[0%strvar4.length];
        if(shouldBailout){
          return  'somestring'
        }
        WScript.Echo(strvar7 <=((+ protoObj1.prop1) < (argMath181 ? argMath180 : argMath181)));
        var strvar9 = strvar6;
        return ;
      }
      static func44 (){
        strvar5 = strvar7[6%strvar7.length];
        argMath178 >>>=([1, 2, 3] instanceof ((typeof Boolean == 'function' ) ? Boolean : Object));
        return -34;
      }
      static func45 (argMath183,...argArr184){
        WScript.Echo(strvar4 >(-- argMath183));
strvar6 +=(argArr184[(((-97 >= 0 ? -97 : 0)) & 0XF)] instanceof ((typeof Function == 'function' ) ? Function : Object));
        argMath178 *=obj0.method0.call(protoObj1 );
        var strvar9 = strvar3;
        strvar9 = strvar9.substring((strvar9.length)/1,(strvar9.length)/4);
        strvar0 = strvar1[6%strvar1.length];
        return -4.07345281674475E+18;
      }
      static func46 (argMath185){
        argMath178 = 501876547.1;
        var strvar9 = strvar4;
        return -2.62875550425758E+17;
      }
      static func47 (){
        return argMath178;
      }
    }
    var reResult2=/(?=bbabb)/g.test('@' + '%#!$');
    return protoObj1.method1.call(class21 );
  }
  static func48 (argMath186,argMath187 = (module4_localbinding_0 = parseInt("-4695088587962236928")),argMath188,argMath189){
    module4_localbinding_0 &=('caller' != ((new module4BaseClass()) >> ('#)!!$' + 'pL,}$#!('.indexOf(((strvar6).replace(strvar6, strvar3).concat(((-730427009.9 ? -1.56116149629499E+18 : argMath186) ? 'caller' : func2.call(arrObj0 , strvar3, arguments[(8)], VarArr0))) + func1.call(obj1 ))))));
    strvar3 = 'b2#S,%ô,Ì,b=²$+'.concat((ui8[1] ? (new module4BaseClass()) : (new module4BaseClass())));
    if(arrObj0[(9)]) {
      class class22 extends module4BaseClass {
        constructor (argMath190 = IntArr1.length){
          super();
(Object.defineProperty(protoObj1, 'prop1', {writable: true, enumerable: false, configurable: true }));
          protoObj1.prop1 = ((protoObj0.prop0 == arrObj0.prop0)&&(h === argMath189));
          strvar3 = strvar0[3%strvar0.length];
          argMath186 |=(arrObj0.prop1 + -2147483648);
          WScript.Echo(strvar2 ==func1.call(protoObj0 ));
        }
        func50 (argMath191,argMath192,argMath193,...argArr194){
          protoObj1.prop1 >>=(991775673, ((('!').replace('!', ('!').replace(/a/g, '!.'+'!!'+'Ü|' + '%Á')) + (strvar0.concat(({prop8: arrObj0.prop1, prop6: 123840879, prop5: argMath186, prop4: argMath193, prop3: argMath189, prop2: 77069475, prop1: obj0.prop1, prop0: argMath192, 84: argMath187} ? (module4_localbinding_1 ? protoObj1.prop1 : f) : (f / (argMath192 == 0 ? 1 : argMath192)))) instanceof ((typeof Function == 'function' ) ? Function : Object))) + (~ (argMath189 >>= (module4_localbinding_1 %= ({prop8: arrObj0.prop1, prop6: 123840879, prop5: argMath186, prop4: argMath193, prop3: argMath189, prop2: 77069475, prop1: obj0.prop1, prop0: argMath192, 84: argMath187} ? (module4_localbinding_1 ? protoObj1.prop1 : f) : (f / (argMath192 == 0 ? 1 : argMath192))))))));
          arrObj0 = protoObj1;
          WScript.Echo(strvar6 !==(argMath187 >> obj1.prop0));
          strvar5 = strvar4[1%strvar4.length];
          var uniqobj20 = Object.create(litObj1);
          if(shouldBailout){
            return  'somestring'
          }
          return f;
        }
        static func51 (...argArr195){
          protoObj1.length= makeArrayLength(ui8[(argArr195) & 255]);
          argMath186 <<=argArr195.length;
          if(shouldBailout){
            return  'somestring'
          }
          return -4.47201164413026E+18;
          WScript.Echo(strvar7 >=i32[(protoObj1.prop0) & 255]);
          return 2014845620;
        }
        static func52 (argMath196,argMath197 = ((argMath196 !== obj0.prop0)||(argMath196 < argMath187)),argMath198 = obj1.method0.call(litObj0 ),argMath199){
          strvar2 = strvar5[3%strvar5.length];
          strvar4 = strvar5[6%strvar5.length];
          var strvar9 = ('ÛW(t.lpfu1³A)+@' + obj1.prop1);
          strvar9 = strvar9.substring((strvar9.length)/2,(strvar9.length)/3);
          strvar4 = strvar5[3%strvar5.length];
          return argMath186;
        }
        static get func53 (){
          module4_localbinding_1 = {prop0: {16: ('¼' + '! #Q'.indexOf((strvar3 + i16[((1157196792.1 * (','.indexOf(strvar0)) - (h && obj0.prop1))) & 255]))), 93: (1157196792.1 * (','.indexOf(strvar0)) - (h && obj0.prop1)), prop0: VarArr0[((((typeof(argMath189)  != 'boolean')  >= 0 ? (typeof(argMath189)  != 'boolean')  : 0)) & 0XF)], prop1: (((argMath186 != argMath189)&&(argMath188 <= argMath186)) ? (argMath186 == module4_localbinding_1) : ((argMath189 <= argMath187)||(protoObj0.prop0 >= a))), prop3: ui16[(136) & 255], prop5: (arrObj0.prop0 /= (({} instanceof ((typeof func3 == 'function' ) ? func3 : Object)) >> (1157196792.1 * (','.indexOf(strvar0)) - (h && obj0.prop1)))), prop6: VarArr0[(11)], prop7: (new module4BaseClass())}};
          strvar3 = strvar5[0%strvar5.length];
          strvar0 = strvar0[5%strvar0.length];
          var strvar9 = strvar1;
          strvar9 = strvar9.substring((strvar9.length)/1,(strvar9.length)/4);
          d %=((new module4BaseClass()) ? (VarArr0.unshift(i32[(97) & 255], arrObj0[(16)], ((new module4BaseClass()) * arrObj0[(9)] - (4.11920927231159E+18 ? 'caller' : (typeof(strvar9)  == 'object') )), obj1.method1.call(litObj0 ), 'caller', (65535 ? (strvar4).replace(/a/g, '^R'+'bB'+'·o' + 'u#') : (parseInt("10312330003331012013123001200000", 4) !== (new module4BaseClass()))), (((new module4BaseClass()) > 'caller') * i32[('caller') & 255] - arguments[(10)]))) : strvar1);
          return obj1.prop1;
        }
      }
      strvar4 = strvar6[5%strvar6.length];
      class class23 {
        constructor (argMath200 = ui32[({92: (IntArr0.unshift((true instanceof ((typeof Number == 'function' ) ? Number : Object)), parseInt("-8N", 27)))}) & 255],argMath201 = {92: (IntArr0.unshift((true instanceof ((typeof Number == 'function' ) ? Number : Object)), parseInt("-8N", 27)))},argMath202){
          var strvar9 = (strvar7).replace('Æ' + 'o%!(', 'ÛW(t.lpfu1³A)+@');
          protoObj1 = obj1;
          WScript.Echo(strvar5 <argMath202);
        }
        func55 (argMath203,argMath204 = func1.call(litObj1 ),argMath205,argMath206){
          obj0.prop5 = (argMath187 = argMath186);
          obj0 = obj0;
          strvar6 = strvar0[2%strvar0.length];
          return 2084420864.1;
        }
        set func56 (argMath207 = -1967957886){
          var strvar9 = strvar7.concat((! ((((new module4BaseClass()) ? (ary.shift()) : 8.89535956739498E+18) / (class22.func51(ary) == 0 ? 1 : class22.func51(ary))) ? -132478347.9 : strvar7))).concat((func3.call(protoObj0 ) * protoObj0.method1.call(litObj1 ) + 'caller'));
          strvar9 = strvar9.substring((strvar9.length)/3,(strvar9.length)/1);
          strvar9 = strvar9[3%strvar9.length];
          return arrObj0.prop0;
        }
        func57 (argMath208 = (h * (parseInt("1838656000") - ((e >= arrObj0.prop0)||(c < argMath189)))),argMath209){
          return e;
        }
      }
      if(((obj0.prop0 += (argMath188 += ((new class22((((module4_localbinding_0 < f)||(argMath187 > argMath187)) % Math.asin(-2147483646)))) * ((obj1.method1.call(litObj1 ) | (obj1.prop1 %= (-482910412.9 == argMath186))) - (obj0.prop1 %= (new module4BaseClass())))))) ? (((ui32[0] <= (new class23('caller',(obj1.method0() ? (new class23(argMath189,(-482910412.9 == argMath186),class22)) : (-- protoObj1.prop0)),arrObj0))) ^ (- (argMath186 ^= ((typeof(1852005230)  != 'number')  && (typeof(argMath188)  == null) )))) instanceof ((typeof Error == 'function' ) ? Error : Object)) : (((i8[((new class23((((ui32[0] <= (new class23('caller',(obj1.method0() ? (new class23(argMath189,(-482910412.9 == argMath186),class22)) : (-- protoObj1.prop0)),arrObj0))) ^ (- (argMath186 ^= ((typeof(1852005230)  != 'number')  && (typeof(argMath188)  == null) )))) instanceof ((typeof Error == 'function' ) ? Error : Object)),(argMath188 ? protoObj0.prop0 : 1331528624),obj1))) & 255] ? c : (typeof(f)  == 'object') ) * ((new class22((','.indexOf(strvar5)))) - ((typeof(f)  == 'object')  * 'caller' + ((protoObj0.prop0 !== f) ? 'caller' : (i8[((new class23((((ui32[0] <= (new class23('caller',(obj1.method0() ? (new class23(argMath189,(-482910412.9 == argMath186),class22)) : (-- protoObj1.prop0)),arrObj0))) ^ (- (argMath186 ^= ((typeof(1852005230)  != 'number')  && (typeof(argMath188)  == null) )))) instanceof ((typeof Error == 'function' ) ? Error : Object)),(argMath188 ? protoObj0.prop0 : 1331528624),obj1))) & 255] ? c : (typeof(f)  == 'object') ))))) || ((new module4BaseClass()) - ((i8[((new class23((((ui32[0] <= (new class23('caller',(obj1.method0() ? (new class23(argMath189,(-482910412.9 == argMath186),class22)) : (-- protoObj1.prop0)),arrObj0))) ^ (- (argMath186 ^= ((typeof(1852005230)  != 'number')  && (typeof(argMath188)  == null) )))) instanceof ((typeof Error == 'function' ) ? Error : Object)),(argMath188 ? protoObj0.prop0 : 1331528624),obj1))) & 255] ? c : (typeof(f)  == 'object') ) * ((new class22((','.indexOf(strvar5)))) - ((typeof(f)  == 'object')  * 'caller' + ((protoObj0.prop0 !== f) ? 'caller' : (i8[((new class23((((ui32[0] <= (new class23('caller',(obj1.method0() ? (new class23(argMath189,(-482910412.9 == argMath186),class22)) : (-- protoObj1.prop0)),arrObj0))) ^ (- (argMath186 ^= ((typeof(1852005230)  != 'number')  && (typeof(argMath188)  == null) )))) instanceof ((typeof Error == 'function' ) ? Error : Object)),(argMath188 ? protoObj0.prop0 : 1331528624),obj1))) & 255] ? c : (typeof(f)  == 'object') ))))))))) {
        strvar3 = strvar2[2%strvar2.length];
      }
      else {
        strvar7 = strvar1[2%strvar1.length];
        if(shouldBailout){
          return  'somestring'
        }
        argMath186 = (argMath188 > argMath186);
(Object.defineProperty(protoObj1, 'prop0', {writable: true, enumerable: false, configurable: true }));
        protoObj1.prop0 = protoObj1.method0.call(class23 );
        WScript.Echo(strvar3 >=i16[((new class22((protoObj1.length >>>= argMath188)))) & 255]);
      }
    }
    else {
      class class24 extends module4BaseClass {
        get func58 (){
          var uniqobj21 = arrObj0;
          e = (! ui8[(arrObj0[((((new module4BaseClass()) >= 0 ? (new module4BaseClass()) : 0)) & 0XF)]) & 255]);
          strvar2 = strvar7 + 65537;
          return -903357715;
        }
        static func59 (argMath210,argMath211,argMath212 = (obj0.method0.call(obj1 ) * -1655094123 + '^R'+'bB'+'·o' + 'u#'.split(/([a7][b7]+[a7])[b7]+[b7]|\S\B\W|abb/iyu,7)),...argArr213){
          if(shouldBailout){
            return  'somestring'
          }
          WScript.Echo(strvar3 !==(argMath188 * argMath188 + -1314751949.9));
          arrObj0 = arrObj0;
          return argMath189;
        }
        static func60 (argMath214,argMath215,argMath216 = arrObj0[(14)]){
          f /=(new module4BaseClass());
          WScript.Echo(strvar2 !=obj0.prop1);
          WScript.Echo(strvar7 >='caller');
          argMath214 = (new module4BaseClass());
          arrObj0.prop0 -=-1604276751;
          return -1.11117848040068E+18;
        }
        static func61 (argMath217,argMath218 = ((argMath186 === protoObj0.prop0)&&(obj1.prop0 > d))){
          strvar7 = strvar2[2%strvar2.length];
          argMath188 = (new module4BaseClass());
          if(shouldBailout){
            return  'somestring'
          }
          return -2104661268;
        }
      }
      class24.func59=uic8[(arrObj0.method1.call(obj1 )) & 255];
      class class25 extends module4BaseClass {
        constructor (argMath219,argMath220){
          super();
          ary[((((argMath187 ? arrObj0.method1.call(obj1 ) : (argMath187 * (-952177598 - argMath220))) >= 0 ? (argMath187 ? arrObj0.method1.call(obj1 ) : (argMath187 * (-952177598 - argMath220))) : 0)) & 0XF)] = (~ 98);
          strvar6 = strvar0[3%strvar0.length];
          var strvar9 = strvar3;
          strvar9 = strvar9.substring((strvar9.length)/2,(strvar9.length)/2);
        }
        static func63 (){
          module4_localbinding_0 = (VarArr0.unshift(uic8[(arrObj0.method1.call(obj1 )) & 255], parseInt("B", 20), ((typeof(strvar6)  == null)  <= (argMath187 /= ('caller' ? IntArr0[((((++ b) >= 0 ? (++ b) : 0)) & 0XF)] : ('caller' * 2)))), i32[(obj0.method0()) & 255], (class24.func59 ^= ((function () {;}) instanceof ((typeof EvalError == 'function' ) ? EvalError : Object))), (++ obj0.prop1), i16[(-8.74083635138156E+18) & 255], (({} instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)) instanceof ((typeof Number == 'function' ) ? Number : Object)), ((new Array()) instanceof ((typeof Array == 'function' ) ? Array : Object)), (true instanceof ((typeof Boolean == 'function' ) ? Boolean : Object)), (f >>= ary[(15)])));
          WScript.Echo(strvar2 <(- obj1.prop1));
          return a;
        }
        static func64 (argMath221,argMath222){
          strvar4 = strvar4 + argMath221;
          argMath222 =2147483650;
          obj1.prop0 <<=('caller' | (class24.func59 += func4.call(obj1 , (typeof(argMath187)  != 'undefined') )));
          strvar4 = strvar1[3%strvar1.length];
          return 5.77436765456407E+18;
        }
        static func65 (argMath223 = (new class24()),argMath224 = Math.pow('caller', arrObj0[(((obj1.method0.call(arrObj0 ) >= 0 ? obj1.method0.call(arrObj0 ) : 0)) & 0XF)]),argMath225 = uic8[(arrObj0.method1.call(obj1 )) & 255],...argArr226){
          var uniqobj22 = {11: (! (((argMath188 < (Object.create({41: 2147483647, 90: 1214673142, prop1: class24.func59, prop2: argMath224, prop3: obj0.prop0, prop5: argMath225}, {}) ? (argMath186 * 86 - class24.func59) : (protoObj0.prop0 = argMath188))) % (ary[(2)] || VarArr0[(17)])) ? (protoObj0.prop1 > (argMath225 *= (func0.call(litObj0 ) * arrObj0.method1.call(litObj1 ) + (-1610352616 ? class24.func59 : 2.23855878086538E+18)))) : (('Ôv'+'^#'+'ÉU' + '!)'.concat(-1460044419.9) + class24.func61.call(protoObj0 , leaf, ui8[1])) > ((function () {;}) instanceof ((typeof EvalError == 'function' ) ? EvalError : Object))))), prop0: (new module4BaseClass()), prop1: 'caller', prop2: argMath225, prop3: (class24.func59 %= Object.create(litObj1, {}))};
          arrObj0 = obj1;
          strvar3 = '-RÜ#$' + '!«((*nf(' + {28: uniqobj22.prop1, 58: protoObj1.prop0, prop2: uniqobj22.prop0, prop3: argMath224, prop4: -492062232, prop5: 9.20613290879192E+18, prop6: 1311107097, prop7: 139};
          class24 = class24;
          var uniqobj23 = {lf0: {prop0: -2147483646, prop1: -1030895243, length: 173374685 , method0: arrObj0.method1, method1: obj1.method1}, lf1: {prop0: -6.23714686287706E+18, prop1: 8.73821929092126E+18, length: 1.21041687954596E+18 , method0: arrObj0.method1, method1: arrObj0.method0}, nd2: {lf0: {prop0: 1016137651, prop1: 600398910, length: 1 , method0: obj0.method0, method1: obj1.method0}}, nd3: {lf0: {prop0: -62, prop1: -8.27867483225962E+18, length: -974914492.9 , method0: func4, method1: arrObj0.method1}, nd1: {lf0: {prop0: 3, prop1: 515088676, length: -1041026122.9 , method0: obj0.method0, method1: func4}, lf1: {prop0: 130446238, prop1: 119, length: -259226056.9 , method0: arrObj0.method0, method1: func0}}}};
          strvar4 = '!'.concat(((new Error('abc')) instanceof ((typeof Boolean == 'function' ) ? Boolean : Object)));
          return arrObj0.prop1;
        }
        static func66 (argMath227 = arrObj0.method1.call(obj1 )){
          return argMath189;
        }
      }
      // Runs interpreter only code
      if(!runningJITtedCode)
      {
        arrObj0.length=(protoObj1.prop1 = arrObj0.method0.call(obj0 ));
      }
      
      return 1093025665.1;
    }
    var __loopvar2 = loopInvariant - 13,__loopSecondaryVar2_0 = loopInvariant;
    (function (__loopvar2,__loopSecondaryVar2_0,loopInvariant) {

      LABEL0: 
      while((ui8[(31) & 255])) {
        if (__loopvar2 > loopInvariant) break;
        __loopvar2 += 4;
        __loopSecondaryVar2_0 -= 4;
        class class26 {
          constructor (argMath228,argMath229,argMath230,argMath231){
          }
          func68 (argMath232 = (typeof(strvar7)  != 'boolean') ,argMath233 = ('¼' + '! #Q'.indexOf(strvar6)),argMath234,...argArr235){
            return -1.94134441138806E+18;
            obj1 = protoObj0;
            return arrObj0.prop1;
          }
          static set func69 (argMath236){
            return -118324706.9;
          }
          static func70 (argMath237,argMath238,argMath239){
(Object.defineProperty(obj0, 'length', {writable: true, enumerable: false, configurable: true }));
            obj0.length = makeArrayLength((((argMath239 <= protoObj1.prop0)||(arrObj0.prop1 > argMath188)) * protoObj1.method1.call(obj0 ) - (protoObj0.prop1 /= (((new EvalError()) instanceof ((typeof func3 == 'function' ) ? func3 : Object)) ? 'caller' : (-2147483648 || (((new RangeError()) instanceof ((typeof String == 'function' ) ? String : Object)) * (typeof(b)  != 'boolean') ))))));
            argMath239 =(ary[(((('ÞÚE$E' + 'b(V$$!ôß'.indexOf(strvar4)) >= 0 ? ('ÞÚE$E' + 'b(V$$!ôß'.indexOf(strvar4)) : 0)) & 0XF)] ? protoObj1.method1.call(obj0 ) : ui8[(31) & 255]);
            return argMath186;
          }
          static func71 (argMath240,argMath241){
            if(shouldBailout){
              return  'somestring'
            }
            strvar2 = strvar4[4%strvar4.length];
            argMath240 = ((typeof(argMath188)  != null)  == f32[((~ ((new Array()) instanceof ((typeof Function == 'function' ) ? Function : Object)))) & 255]);
            return -185;
          }
        }
      }    } )(__loopvar2,__loopSecondaryVar2_0,loopInvariant);

    class class27 extends module4BaseClass {
      constructor (){
        super();
        obj1.prop1 >>>=(new module4BaseClass());
        var strvar9 = 'ÛW(t.lpfu1³A)+@';
        WScript.Echo(strvar2 !=(new module4BaseClass()));
        argMath189 = (FloatArr0.reverse());
      }
      func73 (argMath242 = ((((undefined instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)) ? obj0.prop1 : (obj1.length >>>= (undefined instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)))) ? i8[(IntArr0[((((module4_localbinding_1 = (arrObj0.method0.call(protoObj0 ) < (typeof(argMath188)  == 'string') )) >= 0 ? (module4_localbinding_1 = (arrObj0.method0.call(protoObj0 ) < (typeof(argMath188)  == 'string') )) : 0)) & 0XF)]) & 255] : (arrObj0.method0.call(protoObj0 ) >> (~ ((arrObj0.method0.call(protoObj0 ) < (typeof(argMath188)  == 'string') ) / ((new module4BaseClass()) == 0 ? 1 : (new module4BaseClass())))))) ? i32[(110) & 255] : arrObj0.method1.call(obj1 )),...argArr243){
        obj0.prop4 *=i32[(110) & 255];
        var strvar9 = '#,$òq' + '<¥r!3+#-'.concat(argArr243[(0)]);
        return argMath242;
      }
    }
    return protoObj1.method0.call(litObj1 );
  }
}
export { default as module4_exportbinding_0, default as module4_exportbinding_1 } from 'module2_4c1abe23-d6d6-48ac-9650-608cb0c4a32d.js';
class class28 extends class16 {
  constructor (argMath244){
    super();
    GiantPrintArray.push('obj1.prop0 = ' + (obj1.prop0));
  }
  func75 (argMath245,argMath246,argMath247){
    WScript.Echo(strvar5 >=(typeof arrObj0[((((typeof (argMath245 -= -1658205653.9)) >= 0 ? (typeof (argMath245 -= -1658205653.9)) : 0)) & 0XF)]));
    if(shouldBailout){
      return  'somestring'
    }
    var reResult3='^R'+'bB'+'·o' + 'u#'.split(/abbb(?!aab)|.(\b.\S*\b.)\S*[a7]\w{4}[b7](?=\w{8})\w$/iy,6);
    class class29 {
      constructor (){
        argMath246 = obj1.method1.call(protoObj1 );
        strvar0 = '!.'+'F.'+'g!' + '$*' + ((g >= -2064249189.9) * ((new module4BaseClass()) - (argMath247 ? 38 : -4.97124300953552E+18)));
        WScript.Echo(strvar4 ==obj1.method1.call(protoObj1 ));
        var strvar9 = 'b2#S,%ô,Ì,b=²$+';
        strvar9 = strvar9.substring((strvar9.length)/3,(strvar9.length)/2);
      }
      static func77 (argMath248 = ((new Array()) instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)),argMath249 = 'b2#S,%ô,Ì,b=²$+'.replace('Ö','b2#S,%ô,Ì,b=²$+'),argMath250,argMath251){
        return -987903186;
      }
      static func78 (argMath252 = arrObj0.method1.call(protoObj0 ),argMath253,argMath254,...argArr255){
        strvar7 = strvar1[5%strvar1.length];
        argMath246 = IntArr1[((((argMath246 /= obj1.prop0) >= 0 ? (argMath246 /= obj1.prop0) : 0)) & 0XF)];
        return 454812797;
      }
      static get func79 (){
        obj0.prop1 =(-5.5845026863556E+18 & -723326498);
        if(shouldBailout){
          return  'somestring'
        }
        return 503675242;
      }
      static func80 (){
(Object.defineProperty(obj1, 'prop0', {writable: true, enumerable: false, configurable: true }));
        obj1.prop0 = {prop1: ((protoObj0.length >>= (class16.func29.call(protoObj1 , 'caller'))) * -2147483648 + ((protoObj1.length >>>= 'caller') ? 'caller' : 'caller')), 9: (g != argMath246), 61: c, 50: (argMath247 >>= f32[(117) & 255])};
        argMath245 = (VarArr0.shift());
        var uniqobj24 = {prop0: (new class16()), prop1: ((protoObj0.length >>= (class16.func29.call(protoObj1 , 'caller'))) * -2147483648 + ((protoObj1.length >>>= 'caller') ? 'caller' : 'caller')), prop2: (func1() instanceof ((typeof String == 'function' ) ? String : Object)), prop3: (new module4BaseClass()), prop4: ((new module4BaseClass()) instanceof ((typeof Error == 'function' ) ? Error : Object)), prop5: (((module4_localbinding_1 !== obj0.prop0)||(argMath247 > obj0.prop0)) * (func1() instanceof ((typeof String == 'function' ) ? String : Object)) + (f -= IntArr1[(16)])), prop6: (argMath246 == c), prop7: ui16[((((true instanceof ((typeof RegExp == 'function' ) ? RegExp : Object)) * (++ arrObj0.prop0) - (IntArr0.splice(7,8, arrObj0[(8)], arrObj0[(10)], (((function () {;}) instanceof ((typeof Object == 'function' ) ? Object : Object)) << (typeof(strvar0)  != null) ), FloatArr0[(((obj1.prop0.prop1 >= 0 ? obj1.prop0.prop1 : 0)) & 0XF)], ((+ ((arrObj0.prop1 == obj0.prop0)||(argMath246 != arrObj0.prop0))) < arrObj0.method1.call(protoObj0 )), (VarArr0.pop()), (typeof(('#)!!$' + 'pL,}$#!(').replace('#)!!$' + 'pL,}$#!(', 'Ö'))  != 'boolean') , ('caller' * ((903512746 instanceof ((typeof Function == 'function' ) ? Function : Object)) === (~ -7.41309316977882E+18)) + ((+ ((arrObj0.prop1 == obj0.prop0)||(argMath246 != arrObj0.prop0))) < arrObj0.method1.call(protoObj0 ))), arrObj0.method0.call(obj1.prop0 )))) != (typeof(obj1.prop0.prop1)  == 'undefined') )) & 255]};
        strvar6 = '#)!!$' + 'pL,}$#!(' + (uic8[(4) & 255] << ((function () {;}) instanceof ((typeof Object == 'function' ) ? Object : Object)));
        return b;
      }
    }
    return obj0.prop4;
  }
  static set func81 (argMath256){
    return ((protoObj0.prop1 / (parseInt("HE92AA28623G08", 18) == 0 ? 1 : parseInt("HE92AA28623G08", 18))) - (func0() === ((new Array()) instanceof ((typeof Function == 'function' ) ? Function : Object))));
  }
}
WScript.Echo('a = ' + (a|0));
WScript.Echo('b = ' + (b|0));
WScript.Echo('c = ' + (c|0));
WScript.Echo('d = ' + (d|0));
WScript.Echo('e = ' + (e|0));
WScript.Echo('f = ' + (f|0));
WScript.Echo('g = ' + (g|0));
WScript.Echo('h = ' + (h|0));
WScript.Echo('module4_localbinding_0 = ' + (module4_localbinding_0|0));
WScript.Echo('module4_localbinding_1 = ' + (module4_localbinding_1|0));
WScript.Echo('obj0.prop0 = ' + (obj0.prop0|0));
WScript.Echo('obj0.prop1 = ' + (obj0.prop1|0));
WScript.Echo('obj0.length = ' + (obj0.length|0));
WScript.Echo('protoObj0.prop0 = ' + (protoObj0.prop0|0));
WScript.Echo('protoObj0.prop1 = ' + (protoObj0.prop1|0));
WScript.Echo('protoObj0.length = ' + (protoObj0.length|0));
WScript.Echo('obj1.prop0 = ' + (obj1.prop0|0));
WScript.Echo('obj1.prop1 = ' + (obj1.prop1|0));
WScript.Echo('obj1.length = ' + (obj1.length|0));
WScript.Echo('protoObj1.prop0 = ' + (protoObj1.prop0|0));
WScript.Echo('protoObj1.prop1 = ' + (protoObj1.prop1|0));
WScript.Echo('protoObj1.length = ' + (protoObj1.length|0));
WScript.Echo('arrObj0.prop0 = ' + (arrObj0.prop0|0));
WScript.Echo('arrObj0.prop1 = ' + (arrObj0.prop1|0));
WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
WScript.Echo('obj0.prop4 = ' + (obj0.prop4|0));
WScript.Echo('class28.func81 = ' + (class28.func81|0));
WScript.Echo('strvar0 = ' + (strvar0));
WScript.Echo('strvar1 = ' + (strvar1));
WScript.Echo('strvar2 = ' + (strvar2));
WScript.Echo('strvar3 = ' + (strvar3));
WScript.Echo('strvar4 = ' + (strvar4));
WScript.Echo('strvar5 = ' + (strvar5));
WScript.Echo('strvar6 = ' + (strvar6));
WScript.Echo('strvar7 = ' + (strvar7));
WScript.Echo('arrObj0[0] = ' + (arrObj0[0]|0));
WScript.Echo('arrObj0[1] = ' + (arrObj0[1]|0));
WScript.Echo('arrObj0[2] = ' + (arrObj0[2]|0));
WScript.Echo('arrObj0[3] = ' + (arrObj0[3]|0));
WScript.Echo('arrObj0[4] = ' + (arrObj0[4]|0));
WScript.Echo('arrObj0[5] = ' + (arrObj0[5]|0));
WScript.Echo('arrObj0[6] = ' + (arrObj0[6]|0));
WScript.Echo('arrObj0[7] = ' + (arrObj0[7]|0));
WScript.Echo('arrObj0[8] = ' + (arrObj0[8]|0));
WScript.Echo('arrObj0[9] = ' + (arrObj0[9]|0));
WScript.Echo('arrObj0[10] = ' + (arrObj0[10]|0));
WScript.Echo('arrObj0[11] = ' + (arrObj0[11]|0));
WScript.Echo('arrObj0[12] = ' + (arrObj0[12]|0));
WScript.Echo('arrObj0[13] = ' + (arrObj0[13]|0));
WScript.Echo('arrObj0[14] = ' + (arrObj0[14]|0));
WScript.Echo('arrObj0[arrObj0.length-1] = ' + (arrObj0[arrObj0.length-1]|0));
WScript.Echo('arrObj0.length = ' + (arrObj0.length|0));
WScript.Echo('ary[0] = ' + (ary[0]|0));
WScript.Echo('ary[1] = ' + (ary[1]|0));
WScript.Echo('ary[2] = ' + (ary[2]|0));
WScript.Echo('ary[3] = ' + (ary[3]|0));
WScript.Echo('ary[4] = ' + (ary[4]|0));
WScript.Echo('ary[5] = ' + (ary[5]|0));
WScript.Echo('ary[6] = ' + (ary[6]|0));
WScript.Echo('ary[7] = ' + (ary[7]|0));
WScript.Echo('ary[8] = ' + (ary[8]|0));
WScript.Echo('ary[9] = ' + (ary[9]|0));
WScript.Echo('ary[10] = ' + (ary[10]|0));
WScript.Echo('ary[11] = ' + (ary[11]|0));
WScript.Echo('ary[12] = ' + (ary[12]|0));
WScript.Echo('ary[13] = ' + (ary[13]|0));
WScript.Echo('ary[14] = ' + (ary[14]|0));
WScript.Echo('ary[ary.length-1] = ' + (ary[ary.length-1]|0));
WScript.Echo('ary.length = ' + (ary.length|0));
for (var i = 0; i < GiantPrintArray.length; i++) {
  WScript.Echo(GiantPrintArray[i]);
  }
;
WScript.Echo('sumOfary = ' +  ary.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_ary = ' +  ary.slice(0, 11));;
WScript.Echo('sumOfIntArr0 = ' +  IntArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_IntArr0 = ' +  IntArr0.slice(0, 11));;
WScript.Echo('sumOfIntArr1 = ' +  IntArr1.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_IntArr1 = ' +  IntArr1.slice(0, 11));;
WScript.Echo('sumOfFloatArr0 = ' +  FloatArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_FloatArr0 = ' +  FloatArr0.slice(0, 11));;
WScript.Echo('sumOfVarArr0 = ' +  VarArr0.slice(0, 23).reduce(function(prev, curr) {{ return '' + prev + curr; }},0));
  WScript.Echo('subset_of_VarArr0 = ' +  VarArr0.slice(0, 11));;
`);
WScript.LoadScriptFile('module4_e548fb48-aed8-443c-8b33-45dee06705f5.js', 'module');

// === Output ===
// command: \\chakrafs\fs\Builds\ChakraFull\unreleased\rs2\1607\00005.64965_160711.1740_akroshg_c0a01d67f725b8942f0413ebac9f0b6f9297ec2a\bin\x86_test\JsHost.exe -es6module -maxinterpretcount:5 -maxsimplejitruncount:3 -bgjit- -loopinterpretcount:1 step151.js
// exitcode: C0000005
// stdout:
// 
// stderr:
// FATAL ERROR: jshost.exe failed due to exception code c0000005
// 
// 
// 

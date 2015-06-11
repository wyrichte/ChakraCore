var PolyFuncArr = [];
function GetPolymorphicFunction() {
    if(PolyFuncArr.length > 1) {
        var myFunc = PolyFuncArr.shift();
        PolyFuncArr.push(myFunc);
        return myFunc;
    }
    else {
        return PolyFuncArr[0];
    }
}
function GetObjectwithPolymorphicFunction() {
    var obj = {};
    obj.polyfunc = GetPolymorphicFunction();
    return obj
}
function InitPolymorphicFunctionArray() {
    for(var i = 0; i < arguments.length; i++) {
        PolyFuncArr.push(arguments[i])
    }
}
function test0() {
    function makeArrayLength(x) { if(x < 1 || x > 4294967295 || x != x || isNaN(x) || !isFinite(x)) return 100; else return Math.floor(x) & 0xffff; };;
    function leaf() { return 100; };
    var obj0 = {};
    var obj1 = {};
    var litObj0 = { prop1: 3.14159265358979 };
    var litObj1 = { prop0: 0, prop1: 1 };
    var arrObj0 = {};
    var func0 = function() {
    }
    var func1 = function(argObj0, argStr1) {
    }
    var func2 = function() {
    }
    obj0.method0 = func0;
    var ary = new Array(10);
    var i8 = new Int8Array(256);
    var i16 = new Int16Array(256);
    var i32 = new Int32Array(256);
    var ui8 = new Uint8Array(256);
    var ui16 = new Uint16Array(256);
    var ui32 = new Uint32Array(256);
    var f32 = new Float32Array(256);
    var f64 = new Float64Array(256);
    var cpa8 = WScript.CreateCanvasPixelArray(ui8);;
    var IntArr0 = 1;
    var IntArr1 = 1;
    var FloatArr0 = 1;
    var VarArr0 = 1;
    var a = 1;
    var b = 1;
    var c = 1;
    var d = 1;
    var e = 1;
    var f = 1;
    var g = 1;
    var h = 1;
    var strvar0 = 1;
    var strvar1 = 'x' + 'wdhg';
    var strvar2 = 1;
    var strvar3 = 1;
    var strvar4 = 1;
    var strvar5 = 1;
    var strvar6 = 1;
    var strvar7 = 1;
    var aliasOfobj0 = obj0;;
    var aliasOfobj1 = obj1;;
    var aliasOfarrObj0 = arrObj0;;
    var aliasOflitObj0 = litObj0;;
    var aliasOfary = ary;;
    var aliasOfIntArr0 = IntArr0;;
    var aliasOfIntArr1 = IntArr1;;
    var aliasOfFloatArr0 = FloatArr0;;
    var aliasOfVarArr0 = VarArr0;;
    var aliasOfi16 = i16;
    var aliasOfi32 = i32;
    var aliasOfui8 = ui8;
    function bar0(argMath2) {
    }
    function bar1(argMath3) {
    }
    function bar2(argMath4) {
    }
    function bar3(argMath5) {
    }
    function bar4(argMath6) {
    }
    var __polyobj = GetObjectwithPolymorphicFunction();;
    var __loopvar1 = 0;
    for(var strvar0 in i8) {
        if(strvar0.indexOf('method') != -1) continue;
        if(__loopvar1++ > 3) break;
        i8[strvar0] = (aliasOfui8[(aliasOfobj0.method0.call(aliasOfobj1)) & 255] * 'ahxpr' + 'wlgjmjoy'.concat(strvar1.concat(1).concat(aliasOfui8[(aliasOfobj0.method0.call(aliasOfobj1)) & 255])));
        if(1) {
            obj0.prop0 -= aliasOfui8.length;
            break;
        }
        else {
            arrObj0.length = makeArrayLength(1);
        }
    }
    var id29 = aliasOfobj0.method0.call(aliasOfobj0);
};
test0();
test0();

WScript.Echo("pass");

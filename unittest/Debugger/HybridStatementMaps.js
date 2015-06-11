// Test for proper statement maps when we have to look at SmallSpanSequence::pActualOffsetList

//Switches: -targeted jshost -debuglaunch:hybrid -maxinterpretcount:2 -EnableJitInHybridDebugging

function test0() {
    var obj0 = {};
    var obj1 = {};
    var arrObj0 = {};
    var func0 = function (argObj0, argFunc1, argArrObj2, argFunc3) {}
    arrObj0.method0 = func0;
    var ary = new Array(10);
    var ui8 = new Uint8Array(256);
    var f64 = new Float64Array(256);
    var cpa8 = WScript.CreateCanvasPixelArray(ui8); ;
    var IntArr1 = 1;
    var FloatArr0 = 1;
    var VarArr0 = 1;
    var f = 1;
    function bar1(argObj5) {}
    function bar2(argObj6) {
        h = 1; /**bp:stack();**/
    }
    function bar3(argObj7) {
        var litObj5 = {
            prop0 : 1,
            prop1 : (argObj7.prop1 ? ((argObj7.prop0 === obj0.length) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) : (((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) == 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) * ((((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) == 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) >= ary[((((obj1.prop1 * (1 + arrObj0.prop0)) >= 0 ? (obj1.prop1 * (1 + arrObj0.prop0)) : 0)) & 0XF)]) * ((1 < (f64[(1) & 255] == 1)) - (((f === obj1.prop0) && (a !== c)) == (typeof(argObj7.length) != 1)))) + (FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))),
            prop2 : 1,
            prop3 : 1,
            prop4 : 1,
            prop5 : 1,
            prop6 : (((argObj7.prop1 ? ((argObj7.prop0 === obj0.length) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) : (((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) == 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) * ((((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) != 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) >= ary[((((obj1.prop1 * (1 + arrObj0.prop0)) >= 0 ? (obj1.prop1 * (1 + arrObj0.prop0)) : 0)) & 0XF)]) * ((1 < (f64[(1) & 255] == 1)) - (((f === obj1.prop0) && (a !== c)) == (typeof(argObj7.length) == 1)))) + (FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? 1 : (argObj7.prop1 ? ((argObj7.prop0 === obj0.length) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) : (((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) != 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) * ((((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) != 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) >= ary[((((obj1.prop1 * (1 + arrObj0.prop0)) >= 0 ? (obj1.prop1 * (1 + arrObj0.prop0)) : 0)) & 0XF)]) * ((1 < (f64[(1) & 255] == 1)) - (((f === obj1.prop0) && (a !== c)) == (typeof(argObj7.length) != 1)))) + (FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1))))) * cpa8[(VarArr0[(1)]) & 255] - (1 ? IntArr1[(((bar1.call(arrObj0, 1) >= 0 ? bar1.call(arrObj0, 1) : 0)) & 0XF)] : 1)),
            prop7 : (((argObj7.prop1 ? ((argObj7.prop0 === obj0.length) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) : (((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) != 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) * ((((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) == 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) >= ary[((((obj1.prop1 * (1 + arrObj0.prop0)) >= 0 ? (obj1.prop1 * (1 + arrObj0.prop0)) : 0)) & 0XF)]) * ((1 < (f64[(1) & 255] == 1)) - (((f === obj1.prop0) && (a !== c)) == (typeof(argObj7.length) != 1)))) + (FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? 1 : (argObj7.prop1 ? ((argObj7.prop0 === obj0.length) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) : (((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) != 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) * ((((arrObj0.method0.call(argObj7, 1, 1, 1, 1) * (obj0.length = (typeof(d) != 1)) - ((FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1)))) ? f64[((arrObj0.length > 1)) & 255] : ((obj1.prop1 === obj1.prop0) && (f < argObj7.prop1))) >= ary[((((obj1.prop1 * (1 + arrObj0.prop0)) >= 0 ? (obj1.prop1 * (1 + arrObj0.prop0)) : 0)) & 0XF)]) * ((1 < (f64[(1) & 255] == 1)) - (((f === obj1.prop0) && (a !== c)) == (typeof(argObj7.length) == 1)))) + (FloatArr0[(1)] ? (obj0.prop1 += (d -= 1)) : (--obj1.prop1))))) * cpa8[(VarArr0[(1)]) & 255] - (1 ? IntArr1[(((bar1.call(arrObj0, 1) >= 0 ? bar1.call(arrObj0, 1) : 0)) & 0XF)] : 1))
        };
        bar2.call(obj0, 1);
    }
    bar3.call(obj1, 1);
};
test0();
test0();
test0();

WScript.Echo("done");

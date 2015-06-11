WScript.LoadScriptFile("util.js");

function test0(arr) {
    return arr[0] += 0.1;
};
var arr = WScript.CreateCanvasPixelArray([0, 0, 0, 0]);
test0(arr);
test0(arr);

function test1(arr) {
    arr[0] = 0;
    arr[0] -= 0.9;
    return arr[0];
};
WScript.Echo("test1: " + test1(arr));
WScript.Echo("test1: " + test1(arr));

WScript.Echo("test2");
testSetWithInt(-1, 2, WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]));
testSetWithFloat(-1, 2, WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]));
testSetWithObj(-1, 2, WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]));

WScript.Echo("test2 JIT");
testSetWithInt(-1, 2, WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]));
testSetWithFloat(-1, 2, WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]));
testSetWithObj(-1, 2, WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]), WScript.CreateCanvasPixelArray([0, 0, 0, 0]));

WScript.Echo("test3");
testIndexValueForSet(WScript.CreateCanvasPixelArray([0, 0, 0, 0]));
WScript.Echo("test3 JIT");
testIndexValueForSet(WScript.CreateCanvasPixelArray([0, 0, 0, 0]));
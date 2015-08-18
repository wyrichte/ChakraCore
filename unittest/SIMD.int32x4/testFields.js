function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct")
    else
        WScript.Echo(">> Fail!")
}
var si = SIMD.Int32x4(10, -20, 30, 0);
function testScalarGetters() {
    WScript.Echo('Int32x4 scalar getters');
    var a = SIMD.Int32x4(1, 2, 3, 4);
    equal(1, SIMD.Int32x4.extractLane(a, 0));
    equal(2, SIMD.Int32x4.extractLane(a, 1));
    equal(3, SIMD.Int32x4.extractLane(a, 2));
    equal(4, SIMD.Int32x4.extractLane(a, 3));
}

function testExtractLane1() {
    WScript.Echo("I4 ExtractLane");

    WScript.Echo(typeof si);
    WScript.Echo(si.toString());

    WScript.Echo(typeof SIMD.Int32x4.extractLane(si, 0));
    WScript.Echo(SIMD.Int32x4.extractLane(si, 0).toString());

    WScript.Echo(typeof SIMD.Int32x4.extractLane(si, 1))
    WScript.Echo(SIMD.Int32x4.extractLane(si, 1).toString());

    WScript.Echo(typeof SIMD.Int32x4.extractLane(si, 2));
    WScript.Echo(SIMD.Int32x4.extractLane(si, 2).toString());

    WScript.Echo(typeof SIMD.Int32x4.extractLane(si, 3));
    WScript.Echo(SIMD.Int32x4.extractLane(si, 3).toString());
}

function testReplaceLane1() {
    WScript.Echo("I4 ReplaceLane");

    WScript.Echo(typeof si);
    WScript.Echo(si.toString());

    var v = SIMD.Int32x4.replaceLane(si, 0, 10)
    WScript.Echo(typeof v);
    WScript.Echo(v.toString());

    v = SIMD.Int32x4.replaceLane(si, 1, 12)
    WScript.Echo(typeof v);
    WScript.Echo(v.toString());

    v = SIMD.Int32x4.replaceLane(si, 2, -30)
    WScript.Echo(typeof v);
    WScript.Echo(v.toString());

    v = SIMD.Int32x4.replaceLane(si, 3, 0)
    WScript.Echo(typeof v);
    WScript.Echo(v.toString());

}

function testSignMask() {
    WScript.Echo('Int32x4 signMask');
    var a = SIMD.Int32x4(-1, -2, -3, -4);
    equal(0xf, a.signMask);
    var b = SIMD.Int32x4(1, 2, 3, 4);
    equal(0x0, b.signMask);
    var c = SIMD.Int32x4(1, -2, -3, 4);
    equal(0x6, c.signMask);
    var d = SIMD.Int32x4(-0, 0, 0, -0);
    equal(0x0, d.signMask);
    var e = SIMD.Int32x4(0, -0, -0, 0);
    equal(0x0, e.signMask);
}

function testVectorGetters() {
    WScript.Echo('Int32x4 vector getters');
    var a = SIMD.Int32x4(4, 3, 2, 1);
    var xxxx = SIMD.Int32x4.swizzle(a, 0, 0, 0, 0);
    var yyyy = SIMD.Int32x4.swizzle(a, 1, 1, 1, 1);
    var zzzz = SIMD.Int32x4.swizzle(a, 2, 2, 2, 2);
    var wwww = SIMD.Int32x4.swizzle(a, 3, 3, 3, 3);
    var wzyx = SIMD.Int32x4.swizzle(a, 3, 2, 1, 0);
    equal(4, SIMD.Int32x4.extractLane(xxxx, 0));
    equal(4, SIMD.Int32x4.extractLane(xxxx, 1));
    equal(4, SIMD.Int32x4.extractLane(xxxx, 2));
    equal(4, SIMD.Int32x4.extractLane(xxxx, 3));
    equal(3, SIMD.Int32x4.extractLane(yyyy, 0));
    equal(3, SIMD.Int32x4.extractLane(yyyy, 1));
    equal(3, SIMD.Int32x4.extractLane(yyyy, 2));
    equal(3, SIMD.Int32x4.extractLane(yyyy, 3));
    equal(2, SIMD.Int32x4.extractLane(zzzz, 0));
    equal(2, SIMD.Int32x4.extractLane(zzzz, 1));
    equal(2, SIMD.Int32x4.extractLane(zzzz, 2));
    equal(2, SIMD.Int32x4.extractLane(zzzz, 3));
    equal(1, SIMD.Int32x4.extractLane(wwww, 0));
    equal(1, SIMD.Int32x4.extractLane(wwww, 1));
    equal(1, SIMD.Int32x4.extractLane(wwww, 2));
    equal(1, SIMD.Int32x4.extractLane(wwww, 3));
    equal(1, SIMD.Int32x4.extractLane(wzyx, 0));
    equal(2, SIMD.Int32x4.extractLane(wzyx, 1));
    equal(3, SIMD.Int32x4.extractLane(wzyx, 2));
    equal(4, SIMD.Int32x4.extractLane(wzyx, 3));
}

testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();


testExtractLane1();
WScript.Echo();
testReplaceLane1();
WScript.Echo();
//testSignMask();
//testSignMask();
//testSignMask();
//testSignMask();
//testSignMask();
//testSignMask();
//testSignMask();
//testSignMask();

//testVectorGetters();
//testVectorGetters();
//testVectorGetters();
//testVectorGetters();
//testVectorGetters();
//testVectorGetters();
//testVectorGetters();

function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testConstructor() {
    WScript.Echo("Constructor");
    equal(SIMD.Int8x16, undefined);
    equal(SIMD.Int8x16(1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4, 1, 2, 3, 4), undefined);
    var a = SIMD.Int8x16("2014/10/10", -0, 127, 126, "2014/10/10", -0, 127, 126, "2014/10/10", -0, 127, 126, "2014/10/10", -0, 127, 126);
    WScript.Echo("a.1: " + SIMD.Int8x16.extractLane(a, 0));
    WScript.Echo("a.2: " + SIMD.Int8x16.extractLane(a, 1));
    WScript.Echo("a.3: " + SIMD.Int8x16.extractLane(a, 2));
    WScript.Echo("a.4: " + SIMD.Int8x16.extractLane(a, 3));
    WScript.Echo("a.5: " + SIMD.Int8x16.extractLane(a, 4));
    WScript.Echo("a.6: " + SIMD.Int8x16.extractLane(a, 5));
    WScript.Echo("a.7: " + SIMD.Int8x16.extractLane(a, 6));
    WScript.Echo("a.8: " + SIMD.Int8x16.extractLane(a, 7));
    WScript.Echo("a.9: " + SIMD.Int8x16.extractLane(a, 8));
    WScript.Echo("a.10: " + SIMD.Int8x16.extractLane(a, 9));
    WScript.Echo("a.11: " + SIMD.Int8x16.extractLane(a, 10));
    WScript.Echo("a.12: " + SIMD.Int8x16.extractLane(a, 11));
    WScript.Echo("a.13: " + SIMD.Int8x16.extractLane(a, 12));
    WScript.Echo("a.14: " + SIMD.Int8x16.extractLane(a, 13));
    WScript.Echo("a.15: " + SIMD.Int8x16.extractLane(a, 14));
    WScript.Echo("a.16: " + SIMD.Int8x16.extractLane(a, 15));

    var b = SIMD.Int8x16(4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1);
    var c = SIMD.Int8x16.check(b);
    equal(c, b);
    equal(SIMD.Int8x16.extractLane(c, 0), SIMD.Int8x16.extractLane(b, 0));
    equal(SIMD.Int8x16.extractLane(c, 1), SIMD.Int8x16.extractLane(b, 1));
    equal(SIMD.Int8x16.extractLane(c, 2), SIMD.Int8x16.extractLane(b, 2));
    equal(SIMD.Int8x16.extractLane(c, 3), SIMD.Int8x16.extractLane(b, 3));
    equal(SIMD.Int8x16.extractLane(c, 4), SIMD.Int8x16.extractLane(b, 4));
    equal(SIMD.Int8x16.extractLane(c, 5), SIMD.Int8x16.extractLane(b, 5));
    equal(SIMD.Int8x16.extractLane(c, 6), SIMD.Int8x16.extractLane(b, 6));
    equal(SIMD.Int8x16.extractLane(c, 7), SIMD.Int8x16.extractLane(b, 7));
    equal(SIMD.Int8x16.extractLane(c, 8), SIMD.Int8x16.extractLane(b, 8));
    equal(SIMD.Int8x16.extractLane(c, 9), SIMD.Int8x16.extractLane(b, 9));
    equal(SIMD.Int8x16.extractLane(c, 10), SIMD.Int8x16.extractLane(b, 10));
    equal(SIMD.Int8x16.extractLane(c, 11), SIMD.Int8x16.extractLane(b, 11));
    equal(SIMD.Int8x16.extractLane(c, 12), SIMD.Int8x16.extractLane(b, 12));
    equal(SIMD.Int8x16.extractLane(c, 13), SIMD.Int8x16.extractLane(b, 13));
    equal(SIMD.Int8x16.extractLane(c, 14), SIMD.Int8x16.extractLane(b, 14));
    equal(SIMD.Int8x16.extractLane(c, 15), SIMD.Int8x16.extractLane(b, 15));

    try {
        var m = SIMD.Int8x16.check(1)
    }
    catch (e) {
        WScript.Echo("Type Error");
    }
}
function testFromFloat32x4Bits() {
    var m = SIMD.float32x4.fromInt8x16Bits(SIMD.Int8x16(0x3F800000, 0x40000000, 0x40400000, 0x40800000));
    var n = SIMD.Int8x16.fromFloat32x4Bits(m);
    WScript.Echo("FromFloat32x4Bits");
    equal(1, n.x);
    equal(2, n.y);
    equal(3, n.z);
    equal(4, n.w);
    var a = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var b = SIMD.Int8x16.fromFloat32x4Bits(a);
    equal(0x3F800000, b.x);
    equal(0x40000000, b.y);
    equal(0x40400000, b.z);
    equal(0x40800000, b.w);
}

testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();

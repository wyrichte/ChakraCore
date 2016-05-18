function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

// SIMDFloat32x4
function testSIMDFloat32x4() {
    WScript.Echo("test SIMDFloat32x4......");
    var b = SIMD.Float32x4(1.1, 2.2, 3.3, 4.4);
    var c = SIMD.Float32x4.check(b);
    equal(c, b);
    equal(SIMD.Float32x4.extractLane(c, 0), SIMD.Float32x4.extractLane(b, 0));
    equal(SIMD.Float32x4.extractLane(c, 1), SIMD.Float32x4.extractLane(b, 1));
    equal(SIMD.Float32x4.extractLane(c, 2), SIMD.Float32x4.extractLane(b, 2));
    equal(SIMD.Float32x4.extractLane(c, 3), SIMD.Float32x4.extractLane(b, 3));

    var d = SIMD.Float32x4(0.0, 0.0, 0.0, 0.0);
    equal(SIMD.Float32x4.extractLane(d, 0), 0.0);
    equal(SIMD.Float32x4.extractLane(d, 1), 0.0);
    equal(SIMD.Float32x4.extractLane(d, 2), 0.0);
    equal(SIMD.Float32x4.extractLane(d, 3), 0.0);

    var e = SIMD.Float32x4.splat(4.5);
    equal(SIMD.Float32x4.extractLane(e, 0), 4.5);
    equal(SIMD.Float32x4.extractLane(e, 1), 4.5);
    equal(SIMD.Float32x4.extractLane(e, 2), 4.5);
    equal(SIMD.Float32x4.extractLane(e, 3), 4.5);

    WScript.Echo("test SIMDFloat32x4 WithLane......");
    var f = SIMD.Float32x4.replaceLane(e, 0, 20.5);
    equal(SIMD.Float32x4.extractLane(f, 0), 20.5);
    var g = SIMD.Float32x4.replaceLane(d, 1, 20.0);
    equal(SIMD.Float32x4.extractLane(g, 1), 20.0);
    var h = SIMD.Float32x4.replaceLane(c, 2, 44.0);
    equal(SIMD.Float32x4.extractLane(h, 2), 44.0);
    var i = SIMD.Float32x4.replaceLane(e, 3, 55.5);
    equal(SIMD.Float32x4.extractLane(i, 3), 55.5);

    testSIMDFloat32x4_conversion();
    testSIMDFloat32x4_BinaryOp();
    testSIMDFloat32x4_UnaryOp();
    testSIMDFloat32x4_CompareOp();
    testSIMDFloat32x4_Swizzle();
    testSIMDFloat32x4_Shuffle();
    testSIMDFloat32x4_Select();
}

function testSIMDFloat32x4_conversion() {
    WScript.Echo("test SIMDFloat32x4 conversions......");

    var o = SIMD.Int32x4(1, 2, 3, 4);
    var p = SIMD.Float32x4.fromInt32x4(o);
    equal(1.0, SIMD.Float32x4.extractLane(p, 0));
    equal(2.0, SIMD.Float32x4.extractLane(p, 1));
    equal(3.0, SIMD.Float32x4.extractLane(p, 2));
    equal(4.0, SIMD.Float32x4.extractLane(p, 3));

    var r = SIMD.Int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000);
    var s = SIMD.Float32x4.fromInt32x4Bits(r);
    equal(1.0, SIMD.Float32x4.extractLane(s, 0));
    equal(2.0, SIMD.Float32x4.extractLane(s, 1));
    equal(3.0, SIMD.Float32x4.extractLane(s, 2));
    equal(4.0, SIMD.Float32x4.extractLane(s, 3));
}

function testSIMDFloat32x4_BinaryOp() {
    WScript.Echo("test SIMDFloat32x4 BinaryOp......");

    var a = SIMD.Float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.Float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.Float32x4.add(a, b);
    equal(14.0, SIMD.Float32x4.extractLane(c, 0));
    equal(23.0, SIMD.Float32x4.extractLane(c, 1));
    equal(32.0, SIMD.Float32x4.extractLane(c, 2));
    equal(41.0, SIMD.Float32x4.extractLane(c, 3));

    var a1 = SIMD.Float32x4(4.0, 3.0, 2.0, 1.0);
    var b1 = SIMD.Float32x4(10.0, 20.0, 30.0, 40.0);
    var c1 = SIMD.Float32x4.sub(a1, b1);
    equal(-6.0, SIMD.Float32x4.extractLane(c1, 0));
    equal(-17.0, SIMD.Float32x4.extractLane(c1, 1));
    equal(-28.0, SIMD.Float32x4.extractLane(c1, 2));
    equal(-39.0, SIMD.Float32x4.extractLane(c1, 3));

    var a2 = SIMD.Float32x4(4.0, 3.0, 2.0, 1.0);
    var b2 = SIMD.Float32x4(10.0, 20.0, 30.0, 40.0);
    var c2 = SIMD.Float32x4.mul(a2, b2);
    equal(40.0, SIMD.Float32x4.extractLane(c2, 0));
    equal(60.0, SIMD.Float32x4.extractLane(c2, 1));
    equal(60.0, SIMD.Float32x4.extractLane(c2, 2));
    equal(40.0, SIMD.Float32x4.extractLane(c2, 3));

    var a3 = SIMD.Float32x4(4.0, 9.0, 8.0, 1.0);
    var b3 = SIMD.Float32x4(2.0, 3.0, 1.0, 0.5);
    var c3 = SIMD.Float32x4.div(a3, b3);
    equal(2.0, SIMD.Float32x4.extractLane(c3, 0));
    equal(3.0, SIMD.Float32x4.extractLane(c3, 1));
    equal(8.0, SIMD.Float32x4.extractLane(c3, 2));
    equal(2.0, SIMD.Float32x4.extractLane(c3, 3));
}

function testSIMDFloat32x4_UnaryOp() {
    WScript.Echo("test SIMDFloat32x4 UnaryOp......");

    var a = SIMD.Float32x4(-4.0, -3.0, -2.0, -1.0);
    var c = SIMD.Float32x4.abs(a);
    equal(4.0, SIMD.Float32x4.extractLane(c, 0));
    equal(3.0, SIMD.Float32x4.extractLane(c, 1));
    equal(2.0, SIMD.Float32x4.extractLane(c, 2));
    equal(1.0, SIMD.Float32x4.extractLane(c, 3));

    var a1 = SIMD.Float32x4(-4.0, -3.0, -2.0, -1.0);
    var c1 = SIMD.Float32x4.neg(a1);
    equal(4.0, SIMD.Float32x4.extractLane(c1, 0));
    equal(3.0, SIMD.Float32x4.extractLane(c1, 1));
    equal(2.0, SIMD.Float32x4.extractLane(c1, 2));
    equal(1.0, SIMD.Float32x4.extractLane(c1, 3));

    var a2 = SIMD.Float32x4(8.0, 4.0, 2.0, -2.0);
    var c2 = SIMD.Float32x4.reciprocalApproximation(a2);
    equal(0.125, SIMD.Float32x4.extractLane(c2, 0));
    equal(0.250, SIMD.Float32x4.extractLane(c2, 1));
    equal(0.5, SIMD.Float32x4.extractLane(c2, 2));
    equal(-0.5, SIMD.Float32x4.extractLane(c2, 3));
}

function testSIMDFloat32x4_CompareOp() {
    WScript.Echo("test SIMDFloat32x4 CompareOp......");
    var m = SIMD.Float32x4(1.0, 2.0, 0.1, 0.001);
    var n = SIMD.Float32x4(2.0, 2.0, 0.001, 0.1);
    var cmp;
    cmp = SIMD.Float32x4.lessThan(m, n);
    equal(true, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 3));

    cmp = SIMD.Float32x4.lessThanOrEqual(m, n);
    equal(true, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 3));

    cmp = SIMD.Float32x4.equal(m, n);
    equal(false, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 3));

    cmp = SIMD.Float32x4.notEqual(m, n);
    equal(true, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 3));

    cmp = SIMD.Float32x4.greaterThanOrEqual(m, n);
    equal(false, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 3));

    cmp = SIMD.Float32x4.greaterThan(m, n);
    equal(false, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 3));
}

function testSIMDFloat32x4_Swizzle() {
    WScript.Echo("test SIMDFloat32x4 Swizzle......");
    var a = SIMD.Float32x4(4.0, 3.0, 2.0, 1.0);
    var xxxx = SIMD.Float32x4.swizzle(a, 0, 0, 0, 0);
    var yyyy = SIMD.Float32x4.swizzle(a, 1, 1, 1, 1);
    var zzzz = SIMD.Float32x4.swizzle(a, 2, 2, 2, 2);
    var wwww = SIMD.Float32x4.swizzle(a, 3, 3, 3, 3);
    var wzyx = SIMD.Float32x4.swizzle(a, 3, 2, 1, 0);
    equal(4.0, SIMD.Float32x4.extractLane(xxxx, 0));
    equal(4.0, SIMD.Float32x4.extractLane(xxxx, 1));
    equal(4.0, SIMD.Float32x4.extractLane(xxxx, 2));
    equal(4.0, SIMD.Float32x4.extractLane(xxxx, 3));
    equal(3.0, SIMD.Float32x4.extractLane(yyyy, 0));
    equal(3.0, SIMD.Float32x4.extractLane(yyyy, 1));
    equal(3.0, SIMD.Float32x4.extractLane(yyyy, 2));
    equal(3.0, SIMD.Float32x4.extractLane(yyyy, 3));
    equal(2.0, SIMD.Float32x4.extractLane(zzzz, 0));
    equal(2.0, SIMD.Float32x4.extractLane(zzzz, 1));
    equal(2.0, SIMD.Float32x4.extractLane(zzzz, 2));
    equal(2.0, SIMD.Float32x4.extractLane(zzzz, 3));
    equal(1.0, SIMD.Float32x4.extractLane(wwww, 0));
    equal(1.0, SIMD.Float32x4.extractLane(wwww, 1));
    equal(1.0, SIMD.Float32x4.extractLane(wwww, 2));
    equal(1.0, SIMD.Float32x4.extractLane(wwww, 3));
    equal(1.0, SIMD.Float32x4.extractLane(wzyx, 0));
    equal(2.0, SIMD.Float32x4.extractLane(wzyx, 1));
    equal(3.0, SIMD.Float32x4.extractLane(wzyx, 2));
    equal(4.0, SIMD.Float32x4.extractLane(wzyx, 3));
}

function testSIMDFloat32x4_Shuffle() {
    WScript.Echo("test SIMDFloat32x4 Shuffle......");
    var a = SIMD.Float32x4(1.0, 2.0, 3.0, 4.0);
    var b = SIMD.Float32x4(5.0, 6.0, 7.0, 8.0);
    var xyxy = SIMD.Float32x4.shuffle(a, b, 0, 1, 4, 5);
    var zwzw = SIMD.Float32x4.shuffle(a, b, 2, 3, 6, 7);
    var xxxx = SIMD.Float32x4.shuffle(a, b, 0, 0, 4, 4);
    equal(1.0, SIMD.Float32x4.extractLane(xyxy, 0));
    equal(2.0, SIMD.Float32x4.extractLane(xyxy, 1));
    equal(5.0, SIMD.Float32x4.extractLane(xyxy, 2));
    equal(6.0, SIMD.Float32x4.extractLane(xyxy, 3));
    equal(3.0, SIMD.Float32x4.extractLane(zwzw, 0));
    equal(4.0, SIMD.Float32x4.extractLane(zwzw, 1));
    equal(7.0, SIMD.Float32x4.extractLane(zwzw, 2));
    equal(8.0, SIMD.Float32x4.extractLane(zwzw, 3));
    equal(1.0, SIMD.Float32x4.extractLane(xxxx, 0));
    equal(1.0, SIMD.Float32x4.extractLane(xxxx, 1));
    equal(5.0, SIMD.Float32x4.extractLane(xxxx, 2));
    equal(5.0, SIMD.Float32x4.extractLane(xxxx, 3));
}

function testSIMDFloat32x4_Select() {
    WScript.Echo("test SIMDFloat32x4 Select......");
    var m = SIMD.Bool32x4(true, true, false, false);
    var t = SIMD.Float32x4(1.0, 2.0, 3.0, 4.0);
    var f = SIMD.Float32x4(5.0, 6.0, 7.0, 8.0);
    var s = SIMD.Float32x4.select(m, t, f);
    equal(1.0, SIMD.Float32x4.extractLane(s, 0));
    equal(2.0, SIMD.Float32x4.extractLane(s, 1));
    equal(7.0, SIMD.Float32x4.extractLane(s, 2));
    equal(8.0, SIMD.Float32x4.extractLane(s, 3));
}

testSIMDFloat32x4();

WScript.StartProfiling(testSIMDFloat32x4);


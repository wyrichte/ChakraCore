function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

// SIMDFloat64x2
function testSIMDFloat64x2() {
    WScript.Echo("test SIMDFloat64x2......");
    var b = SIMD.float64x2(1.1, 2.2);
    var c = SIMD.float64x2.check(b);
    equal(c, b);
    equal(c.x, b.x);
    equal(c.y, b.y);

    WScript.Echo("test SIMDFloat64x2 WithLane......");
    var f = SIMD.float64x2.withX(c, 20.5);
    equal(f.x, 20.5);
    var g = SIMD.float64x2.withY(c, 20.0);
    equal(g.y, 20.0);

    testSIMDFloat64x2_conversion();
    testSIMDFloat64x2_BinaryOp();
    testSIMDFloat64x2_UnaryOp();
    testSIMDFloat64x2_CompareOp();
    testSIMDFloat64x2_Swizzle();
    testSIMDFloat64x2_Shuffle();
    testSIMDFloat64x2_Clamp();
    testSIMDFloat64x2_Select();
}

function testSIMDFloat64x2_conversion() {
    WScript.Echo("test SIMDFloat64x2 conversions......");

    var a = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var b = SIMD.float64x2.fromFloat32x4(a);
    equal(1.0, b.x);
    equal(2.0, b.y);

    var m = SIMD.float32x4.fromInt32x4Bits(SIMD.int32x4(0x00000000, 0x3ff00000, 0x0000000, 0x40000000));
    var n = SIMD.float64x2.fromFloat32x4Bits(m);
    equal(1.0, n.x);
    equal(2.0, n.y);

    var c = SIMD.int32x4(1, 2, 3, 4);
    var d = SIMD.float64x2.fromInt32x4(c);
    equal(1.0, c.x);
    equal(2.0, d.y);

    var e = SIMD.int32x4(0x00000000, 0x3ff00000, 0x0000000, 0x40000000);
    var f = SIMD.float64x2.fromInt32x4Bits(e);
    equal(1.0, n.x);
    equal(2.0, n.y);

}

function testSIMDFloat64x2_BinaryOp() {
    WScript.Echo("test SIMDFloat64x2 BinaryOp......");

    var a = SIMD.float64x2(2.0, 1.0);
    var b = SIMD.float64x2(10.0, 20.0);
    var c = SIMD.float64x2.add(a, b);
    equal(12.0, c.x);
    equal(21.0, c.y);

    var a1 = SIMD.float64x2(2.0, 1.0);
    var b1 = SIMD.float64x2(10.0, 20.0);
    var c1 = SIMD.float64x2.sub(a1, b1);
    equal(-8.0, c1.x);
    equal(-19.0, c1.y);

    var a2 = SIMD.float64x2(2.0, 1.0);
    var b2 = SIMD.float64x2(10.0, 20.0);
    var c2 = SIMD.float64x2.mul(a2, b2);
    equal(20.0, c2.x);
    equal(20.0, c2.y);

    var a3 = SIMD.float64x2(4.0, 9.0);
    var b3 = SIMD.float64x2(2.0, 3.0);
    var c3 = SIMD.float64x2.div(a3, b3);
    equal(2.0, c3.x);
    equal(3.0, c3.y);
}

function testSIMDFloat64x2_UnaryOp()
{
    WScript.Echo("test SIMDFloat64x2 UnaryOp......");
    var a = SIMD.float64x2(-2.0, -1.0);
    var c = SIMD.float64x2.abs(a);
    equal(2.0, c.x);
    equal(1.0, c.y);
    c = SIMD.float64x2.abs(SIMD.float64x2(2.0, 1.0));
    equal(2.0, c.x);
    equal(1.0, c.y);

    var a1 = SIMD.float64x2(-2.0, -1.0);
    var c1 = SIMD.float64x2.neg(a1);
    equal(2.0, c1.x);
    equal(1.0, c1.y);
    c1 = SIMD.float64x2.neg(SIMD.float64x2(2.0, 1.0));
    equal(-2.0, c1.x);
    equal(-1.0, c1.y);

    var a2 = SIMD.float64x2(2.0, -2.0);
    var c2 = SIMD.float64x2.reciprocal(a2);
    equal(0.5, c2.x);
    equal(-0.5, c2.y);

    var a3= SIMD.float64x2(16.0, 9.0);
    var c3 = SIMD.float64x2.sqrt(a3);
    equal(4.0, c3.x);
    equal(3.0, c3.y);
}

function testSIMDFloat64x2_CompareOp()
{
    WScript.Echo("test SIMDFloat64x2 CompareOp......");
    var m = SIMD.float64x2(1.0, 2.0);
    var n = SIMD.float64x2(2.0, 2.0);
    var o = SIMD.float64x2(0.1, 0.001);
    var p = SIMD.float64x2(0.001, 0.1);

    var cmp;
    cmp = SIMD.float64x2.lessThan(m, n);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
    cmp = SIMD.float64x2.lessThan(o, p);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);

    cmp = SIMD.float64x2.lessThanOrEqual(m, n);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);
    cmp = SIMD.float64x2.lessThanOrEqual(o, p);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);

    cmp = SIMD.float64x2.equal(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);
    cmp = SIMD.float64x2.equal(o, p);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);

    cmp = SIMD.float64x2.notEqual(m, n);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
    cmp = SIMD.float64x2.notEqual(o, p);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);

    cmp = SIMD.float64x2.greaterThanOrEqual(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);
    cmp = SIMD.float64x2.greaterThanOrEqual(o, p);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);

    cmp = SIMD.float64x2.greaterThan(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
    cmp = SIMD.float64x2.greaterThan(o, p);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
}

function testSIMDFloat64x2_Swizzle()
{
    WScript.Echo("test SIMDFloat64x2 Swizzle......");
    var a = SIMD.float64x2(1.0, 2.0);
    var xx = SIMD.float64x2.swizzle(a, 0, 0);
    var xy = SIMD.float64x2.swizzle(a, 0, 1);
    var yx = SIMD.float64x2.swizzle(a, 1, 0);
    var yy = SIMD.float64x2.swizzle(a, 1, 1);
    equal(1.0, xx.x);
    equal(1.0, xx.y);
    equal(1.0, xy.x);
    equal(2.0, xy.y);
    equal(2.0, yx.x);
    equal(1.0, yx.y);
    equal(2.0, yy.x);
    equal(2.0, yy.y);
}

function testSIMDFloat64x2_Shuffle()
{
    WScript.Echo("test SIMDFloat64x2 Shuffle......");
    var a = SIMD.float64x2(1.0, 2.0);
    var b  = SIMD.float64x2(3.0, 4.0);
    var xx = SIMD.float64x2.shuffle(a, b, 0, 2);
    var xy = SIMD.float64x2.shuffle(a, b, 0, 3);
    var yx = SIMD.float64x2.shuffle(a, b, 1, 2);
    var yy = SIMD.float64x2.shuffle(a, b, 1, 3);
    equal(1.0, xx.x);
    equal(3.0, xx.y);
    equal(1.0, xy.x);
    equal(4.0, xy.y);
    equal(2.0, yx.x);
    equal(3.0, yx.y);
    equal(2.0, yy.x);
    equal(4.0, yy.y);
}

function testSIMDFloat64x2_Clamp()
{
    WScript.Echo("test SIMDFloat64x2 Clamp......");
    var a = SIMD.float64x2(-20.0, 10.0);
    var b = SIMD.float64x2(2.125, 3.0);
    var lower = SIMD.float64x2(2.0, 1.0);
    var upper = SIMD.float64x2(2.5, 5.0);
    var c = SIMD.float64x2.clamp(a, lower, upper);
    equal(2.0, c.x);
    equal(5.0, c.y);
    c = SIMD.float64x2.clamp(b, lower, upper);
    equal(2.125, c.x);
    equal(3.0, c.y);
    a = SIMD.float64x2(-3.4e200, 3.4e250);
    b = SIMD.float64x2(3.4e100, 3.4e200);
    lower = SIMD.float64x2(3.4e50, 3.4e100);
    upper = SIMD.float64x2(3.4e150, 3.4e300);
    c = SIMD.float64x2.clamp(a, lower, upper);
    equal(3.4e50, c.x);
    equal(3.4e250, c.y);
    c = SIMD.float64x2.clamp(b, lower, upper);
    equal(3.4e100, c.x);
    equal(3.4e200, c.y);
}

function testSIMDFloat64x2_Select()
{
    WScript.Echo("test SIMDFloat64x2 Select......");
    var m = SIMD.int32x4.bool(true, true, false, false);
    var t = SIMD.float64x2(1.0, 2.0);
    var f = SIMD.float64x2(3.0, 4.0);
    var s = SIMD.float64x2.select(m, t, f);
    equal(1.0, s.x);
    equal(4.0, s.y);
}

testSIMDFloat64x2();

WScript.StartProfiling(testSIMDFloat64x2);


function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

// SIMDFloat32x4
function testSIMDFloat32x4() {
    WScript.Echo("test SIMDFloat32x4......");
    var b = SIMD.float32x4(1.1, 2.2, 3.3, 4.4);
    var c = SIMD.float32x4.check(b);
    equal(c, b);
    equal(c.x, b.x);
    equal(c.y, b.y);
    equal(c.z, b.z);
    equal(c.w, b.w);

    var d = SIMD.float32x4.zero();
    equal(d.x, 0.0);
    equal(d.y, 0.0);
    equal(d.z, 0.0);
    equal(d.w, 0.0);

    var e = SIMD.float32x4.splat(4.5);
    equal(e.x, 4.5);
    equal(e.y, 4.5);
    equal(e.z, 4.5);
    equal(e.w, 4.5);

    WScript.Echo("test SIMDFloat32x4 WithLane......");
    var f = SIMD.float32x4.withX(e, 20.5);
    equal(f.x, 20.5);
    var g = SIMD.float32x4.withY(d, 20.0);
    equal(g.y, 20.0);
    var h = SIMD.float32x4.withZ(c, 44.0);
    equal(h.z, 44.0);
    var i = SIMD.float32x4.withW(e, 55.5);
    equal(i.w, 55.5);

    testSIMDFloat32x4_conversion();
    testSIMDFloat32x4_BinaryOp();
    testSIMDFloat32x4_UnaryOp();
    testSIMDFloat32x4_CompareOp();
    testSIMDFloat32x4_Swizzle();
    testSIMDFloat32x4_Shuffle();
    testSIMDFloat32x4_Clamp();
    testSIMDFloat32x4_Select();
}

function testSIMDFloat32x4_conversion() {
    WScript.Echo("test SIMDFloat32x4 conversions......");

    var j = SIMD.float64x2(1.0, 2.0);
    var k = SIMD.float32x4.fromFloat64x2(j);
    equal(1.0, k.x);
    equal(2.0, k.y);
    equal(0.0, k.z);
    equal(0.0, k.w);

    var m = SIMD.float64x2.fromInt32x4Bits(SIMD.int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000));
    var n = SIMD.float32x4.fromFloat64x2Bits(m);
    equal(1.0, n.x);
    equal(2.0, n.y);
    equal(3.0, n.z);
    equal(4.0, n.w);

    var o = SIMD.int32x4(1, 2, 3, 4);
    var p = SIMD.float32x4.fromInt32x4(o);
    equal(1.0, p.x);
    equal(2.0, p.y);
    equal(3.0, p.z);
    equal(4.0, p.w);

    var r = SIMD.int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000);
    var s = SIMD.float32x4.fromInt32x4Bits(r);
    equal(1.0, s.x);
    equal(2.0, s.y);
    equal(3.0, s.z);
    equal(4.0, s.w);
}

function testSIMDFloat32x4_BinaryOp() {
    WScript.Echo("test SIMDFloat32x4 BinaryOp......");

    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.float32x4.add(a, b);
    equal(14.0, c.x);
    equal(23.0, c.y);
    equal(32.0, c.z);
    equal(41.0, c.w);

    var a1 = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b1 = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c1 = SIMD.float32x4.sub(a1, b1);
    equal(-6.0, c1.x);
    equal(-17.0, c1.y);
    equal(-28.0, c1.z);
    equal(-39.0, c1.w);

    var a2 = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b2 = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c2 = SIMD.float32x4.mul(a2, b2);
    equal(40.0, c2.x);
    equal(60.0, c2.y);
    equal(60.0, c2.z);
    equal(40.0, c2.w);

    var a3 = SIMD.float32x4(4.0, 9.0, 8.0, 1.0);
    var b3 = SIMD.float32x4(2.0, 3.0, 1.0, 0.5);
    var c3 = SIMD.float32x4.div(a3, b3);
    equal(2.0, c3.x);
    equal(3.0, c3.y);
    equal(8.0, c3.z);
    equal(2.0, c3.w);

    var a4 = SIMD.float32x4(8.0, 4.0, 2.0, -2.0);
    var c4 = SIMD.float32x4.scale(a4, 0.5);
    equal(4.0, c4.x);
    equal(2.0, c4.y);
    equal(1.0, c4.z);
    equal(-1.0, c4.w);
}

function testSIMDFloat32x4_UnaryOp() {
    WScript.Echo("test SIMDFloat32x4 UnaryOp......");

    var a = SIMD.float32x4(-4.0, -3.0, -2.0, -1.0);
    var c = SIMD.float32x4.abs(a);
    equal(4.0, c.x);
    equal(3.0, c.y);
    equal(2.0, c.z);
    equal(1.0, c.w);

    var a1 = SIMD.float32x4(-4.0, -3.0, -2.0, -1.0);
    var c1 = SIMD.float32x4.neg(a1);
    equal(4.0, c1.x);
    equal(3.0, c1.y);
    equal(2.0, c1.z);
    equal(1.0, c1.w);

    var a2 = SIMD.float32x4(8.0, 4.0, 2.0, -2.0);
    var c2 = SIMD.float32x4.reciprocal(a2);
    equal(0.125, c2.x);
    equal(0.250, c2.y);
    equal(0.5, c2.z);
    equal(-0.5, c2.w);
}

function testSIMDFloat32x4_CompareOp()
{
    WScript.Echo("test SIMDFloat32x4 CompareOp......");
    var m = SIMD.float32x4(1.0, 2.0, 0.1, 0.001);
    var n = SIMD.float32x4(2.0, 2.0, 0.001, 0.1);
    var cmp;
    cmp = SIMD.float32x4.lessThan(m, n);
    equal(-1, cmp.x);
    equal(0x0, cmp.y);
    equal(0x0, cmp.z);
    equal(-1, cmp.w);

    cmp = SIMD.float32x4.lessThanOrEqual(m, n);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(-1, cmp.w);

    cmp = SIMD.float32x4.equal(m, n);
    equal(0x0, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);

    cmp = SIMD.float32x4.notEqual(m, n);
    equal(-1, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);

    cmp = SIMD.float32x4.greaterThanOrEqual(m, n);
    equal(0x0, cmp.x);
    equal(-1, cmp.y);
    equal(-1, cmp.z);
    equal(0x0, cmp.w);

    cmp = SIMD.float32x4.greaterThan(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(0x0, cmp.w);
}

function testSIMDFloat32x4_Swizzle()
{
    WScript.Echo("test SIMDFloat32x4 Swizzle......");
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var xxxx = SIMD.float32x4.swizzle(a, 0, 0, 0, 0);
    var yyyy = SIMD.float32x4.swizzle(a, 1, 1, 1, 1);
    var zzzz = SIMD.float32x4.swizzle(a, 2, 2, 2, 2);
    var wwww = SIMD.float32x4.swizzle(a, 3, 3, 3, 3);
    var wzyx = SIMD.float32x4.swizzle(a, 3, 2, 1, 0);
    equal(4.0, xxxx.x);
    equal(4.0, xxxx.y);
    equal(4.0, xxxx.z);
    equal(4.0, xxxx.w);
    equal(3.0, yyyy.x);
    equal(3.0, yyyy.y);
    equal(3.0, yyyy.z);
    equal(3.0, yyyy.w);
    equal(2.0, zzzz.x);
    equal(2.0, zzzz.y);
    equal(2.0, zzzz.z);
    equal(2.0, zzzz.w);
    equal(1.0, wwww.x);
    equal(1.0, wwww.y);
    equal(1.0, wwww.z);
    equal(1.0, wwww.w);
    equal(1.0, wzyx.x);
    equal(2.0, wzyx.y);
    equal(3.0, wzyx.z);
    equal(4.0, wzyx.w);
}

function testSIMDFloat32x4_Shuffle()
{
    WScript.Echo("test SIMDFloat32x4 Shuffle......");
    var a = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var b = SIMD.float32x4(5.0, 6.0, 7.0, 8.0);
    var xyxy = SIMD.float32x4.shuffle(a, b, 0, 1, 4, 5);
    var zwzw = SIMD.float32x4.shuffle(a, b, 2, 3, 6, 7);
    var xxxx = SIMD.float32x4.shuffle(a, b, 0, 0, 4, 4);
    equal(1.0, xyxy.x);
    equal(2.0, xyxy.y);
    equal(5.0, xyxy.z);
    equal(6.0, xyxy.w);
    equal(3.0, zwzw.x);
    equal(4.0, zwzw.y);
    equal(7.0, zwzw.z);
    equal(8.0, zwzw.w);
    equal(1.0, xxxx.x);
    equal(1.0, xxxx.y);
    equal(5.0, xxxx.z);
    equal(5.0, xxxx.w);
}

function testSIMDFloat32x4_Clamp()
{
    WScript.Echo("test SIMDFloat32x4 Clamp......");
    var a = SIMD.float32x4(-20.0, 10.0, 30.0, 0.5);
    var lower = SIMD.float32x4(2.0, 1.0, 50.0, 0.0);
    var upper = SIMD.float32x4(2.5, 5.0, 55.0, 1.0);
    var c = SIMD.float32x4.clamp(a, lower, upper);
    equal(2.0, c.x);
    equal(5.0, c.y);
    equal(50.0, c.z);
    equal(0.5, c.w);
}

function testSIMDFloat32x4_Select()
{
    WScript.Echo("test SIMDFloat32x4 Select......");
    var m = SIMD.int32x4.bool(true, true, false, false);
    var t = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var f = SIMD.float32x4(5.0, 6.0, 7.0, 8.0);
    var s = SIMD.float32x4.select(m, t, f);
    equal(1.0, s.x);
    equal(2.0, s.y);
    equal(7.0, s.z);
    equal(8.0, s.w);
}

testSIMDFloat32x4();

WScript.StartProfiling(testSIMDFloat32x4);


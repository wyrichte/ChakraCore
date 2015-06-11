function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

// SIMDInt32x4
function testSIMDInt32x4() {
    WScript.Echo("test SIMDInt32x4.....");
    var b = SIMD.int32x4(1, 2, 3, 4);
    var c = SIMD.int32x4(b);
    equal(c, b);
    equal(c.x, b.x);
    equal(c.y, b.y);
    equal(c.z, b.z);
    equal(c.w, b.w);

    WScript.Echo("test SIMDInt32x4 WithLane......");
    var f = SIMD.int32x4.withX(c, 20);
    equal(f.x, 20);
    var g = SIMD.int32x4.withY(c, 22);
    equal(g.y, 22);
    var h = SIMD.int32x4.withZ(c, 44.0);
    equal(h.z, 44.0);
    var i = SIMD.int32x4.withW(c, 55.5);
    equal(i.w, 55.5);

    testSIMDInt32x4_conversion();
    testSIMDInt32x4_BinaryOp();
    testSIMDInt32x4_UnaryOp();
    testSIMDInt32x4_CompareOp();
    testSIMDInt32x4_Shuffle();
    testSIMDInt32x4_ShuffleMix();
    testSIMDInt32x4_Select();
}

function testSIMDInt32x4_conversion()
{
    WScript.Echo("test SIMDInt32x4 Conversion......");
    var m = SIMD.float32x4(1.0, 2.2, 3.6, 4.8);
    var n = SIMD.int32x4.fromFloat32x4(m);
    equal(1, n.x);
    equal(2, n.y);
    equal(3, n.z);
    equal(4, n.w);

    var a = SIMD.float64x2(1.0, 2.2);
    var b = SIMD.int32x4.fromFloat64x2(a);
    equal(1, b.x);
    equal(2, b.y);
    equal(0, b.z);
    equal(0, b.w);

    var c = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var d = SIMD.int32x4.fromFloat32x4Bits(c);
    equal(0x3F800000, d.x);
    equal(0x40000000, d.y);
    equal(0x40400000, d.z);
    equal(0x40800000, d.w);

    var e = SIMD.float64x2(1.0, 2.0);
    var f = SIMD.int32x4.fromFloat64x2Bits(e);
    equal(0x00000000, f.x);
    equal(0x3FF00000, f.y);
    equal(0x00000000, f.z);
    equal(0x40000000, f.w);
}



function testSIMDInt32x4_BinaryOp()
{
    WScript.Echo("test SIMDInt32x4 BinaryOp......");
    var m = SIMD.int32x4(0xAAAAAAAA, 0xAAAAAAAA, -1431655766, 0xAAAAAAAA);
    var n = SIMD.int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    equal(-1431655766, m.x);
    equal(-1431655766, m.y);
    equal(-1431655766, m.z);
    equal(-1431655766, m.w);
    equal(0x55555555, n.x);
    equal(0x55555555, n.y);
    equal(0x55555555, n.z);
    equal(0x55555555, n.w);
    equal(true, n.flagX);
    equal(true, n.flagY);
    equal(true, n.flagZ);
    equal(true, n.flagW);
    var o = SIMD.int32x4.and(m, n);  // and
    equal(0x0, o.x);
    equal(0x0, o.y);
    equal(0x0, o.z);
    equal(0x0, o.w);
    equal(false, o.flagX);
    equal(false, o.flagY);
    equal(false, o.flagZ);
    equal(false, o.flagW);

    var m1 = SIMD.int32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    var n1 = SIMD.int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    var o1 = SIMD.int32x4.or(m1, n1);  // or
    equal(-1, o1.x);
    equal(-1, o1.y);
    equal(-1, o1.z);
    equal(-1, o1.w);
    equal(true, o1.flagX);
    equal(true, o1.flagY);
    equal(true, o1.flagZ);
    equal(true, o1.flagW);

    var m2 = SIMD.int32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    var n2 = SIMD.int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    n2 = SIMD.int32x4.withX(n2, 0xAAAAAAAA);
    n2 = SIMD.int32x4.withY(n2, 0xAAAAAAAA);
    n2 = SIMD.int32x4.withZ(n2, 0xAAAAAAAA);
    n2 = SIMD.int32x4.withW(n2, 0xAAAAAAAA);
    equal(-1431655766, n2.x);
    equal(-1431655766, n2.y);
    equal(-1431655766, n2.z);
    equal(-1431655766, n2.w);
    var o2 = SIMD.int32x4.xor(m2, n2);  // xor
    equal(0x0, o2.x);
    equal(0x0, o2.y);
    equal(0x0, o2.z);
    equal(0x0, o2.w);
    equal(false, o2.flagX);
    equal(false, o2.flagY);
    equal(false, o2.flagZ);
    equal(false, o2.flagW);

    var a = SIMD.int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x7fffffff, 0x0);
    var b = SIMD.int32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c = SIMD.int32x4.add(a, b);
    equal(0x0, c.x);
    equal(-2, c.y);
    equal(-0x80000000, c.z);
    equal(-1, c.w);

    var a1 = SIMD.int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b1 = SIMD.int32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c1 = SIMD.int32x4.sub(a1, b1);
    equal(-2, c1.x);
    equal(0x0, c1.y);
    equal(0x7FFFFFFF, c1.z);
    equal(0x1, c1.w);

    var a2 = SIMD.int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b2 = SIMD.int32x4(0x1, 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF);
    var c2 = SIMD.int32x4.mul(a2, b2);
    equal(-1, c2.x);
    equal(0x1, c2.y);
    equal(0x0, c2.z);
    equal(0x0, c2.w);
}

function testSIMDInt32x4_UnaryOp()
{
    WScript.Echo("test SIMDInt32x4 UnaryOp......");
    var m = SIMD.int32x4(16, 32, 64, 128);
    var n = SIMD.int32x4(-1, -2, -3, -4);
    m = SIMD.int32x4.neg(m);
    n = SIMD.int32x4.neg(n);
    equal(-16, m.x);
    equal(-32, m.y);
    equal(-64, m.z);
    equal(-128, m.w);
    equal(1, n.x);
    equal(2, n.y);
    equal(3, n.z);
    equal(4, n.w);
}

function testSIMDInt32x4_CompareOp()
{
    WScript.Echo("test SIMDInt32x4 CompareOp......");
    WScript.Echo("SIMDInt32x4 lessThan");
    var m = SIMD.int32x4(1000, 2000, 100, 1);
    var n = SIMD.int32x4(2000, 2000, 1, 100);
    var cmp;
    cmp = SIMD.int32x4.lessThan(m, n);
    equal(-1, cmp.x);
    equal(0x0, cmp.y);
    equal(0x0, cmp.z);
    equal(-1, cmp.w);

    WScript.Echo("SIMDInt32x4 equal");
    cmp = SIMD.int32x4.equal(m, n);
    equal(0x0, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);

    WScript.Echo("SIMDInt32x4 greaterThan");
    cmp = SIMD.int32x4.greaterThan(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(0x0, cmp.w);

}

function testSIMDInt32x4_Shuffle()
{
    WScript.Echo("test SIMDInt32x4 Shuffle......");
    var a = SIMD.int32x4(1, 2, 3, 4);
    var xyxy = SIMD.int32x4.shuffle(a, SIMD.XYXY);
    var zwzw = SIMD.int32x4.shuffle(a, SIMD.ZWZW);
    var xxxx = SIMD.int32x4.shuffle(a, SIMD.XXXX);
    equal(1, xyxy.x);
    equal(2, xyxy.y);
    equal(1, xyxy.z);
    equal(2, xyxy.w);
    equal(3, zwzw.x);
    equal(4, zwzw.y);
    equal(3, zwzw.z);
    equal(4, zwzw.w);
    equal(1, xxxx.x);
    equal(1, xxxx.y);
    equal(1, xxxx.z);
    equal(1, xxxx.w);
}

function testSIMDInt32x4_ShuffleMix()
{
    WScript.Echo("test SIMDInt32x4 ShuffleMix......");
    var a = SIMD.int32x4(1, 2, 3, 4);
    var b = SIMD.int32x4(5, 6, 7, 8);
    var xyxy = SIMD.int32x4.shuffleMix(a, b, SIMD.XYXY);
    var zwzw = SIMD.int32x4.shuffleMix(a, b, SIMD.ZWZW);
    var xxxx = SIMD.int32x4.shuffleMix(a, b, SIMD.XXXX);
    equal(1, xyxy.x);
    equal(2, xyxy.y);
    equal(5, xyxy.z);
    equal(6, xyxy.w);
    equal(3, zwzw.x);
    equal(4, zwzw.y);
    equal(7, zwzw.z);
    equal(8, zwzw.w);
    equal(1, xxxx.x);
    equal(1, xxxx.y);
    equal(5, xxxx.z);
    equal(5, xxxx.w);
}

function testSIMDInt32x4_Select()
{
    WScript.Echo("test SIMDInt32x4 Select......");
    var m = SIMD.int32x4.bool(true, true, false, false);
    var t = SIMD.int32x4(1, 2, 3, 4);
    var f = SIMD.int32x4(5, 6, 7, 8);
    var s = SIMD.int32x4.select(m, t, f);
    equal(1, s.x);
    equal(2, s.y);
    equal(7, s.z);
    equal(8, s.w);
}

testSIMDInt32x4();

WScript.StartProfiling(testSIMDInt32x4);


function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

// SIMDInt32x4
function testSIMDInt32x4() {
    WScript.Echo("test SIMDInt32x4.....");
    var b = SIMD.Int32x4(1, 2, 3, 4);
    var c = SIMD.Int32x4.check(b);
    equal(c, b);
    equal(SIMD.Int32x4.extractLane(c, 0), SIMD.Int32x4.extractLane(b, 0));
    equal(SIMD.Int32x4.extractLane(c, 1), SIMD.Int32x4.extractLane(b, 1));
    equal(SIMD.Int32x4.extractLane(c, 2), SIMD.Int32x4.extractLane(b, 2));
    equal(SIMD.Int32x4.extractLane(c, 3), SIMD.Int32x4.extractLane(b, 3));

    WScript.Echo("test SIMDInt32x4 WithLane......");
    var f = SIMD.Int32x4.replaceLane(c, 0, 20);
    equal(SIMD.Int32x4.extractLane(f, 0), 20);
    var g = SIMD.Int32x4.replaceLane(c, 1, 22);
    equal(SIMD.Int32x4.extractLane(g, 1), 22);
    var h = SIMD.Int32x4.replaceLane(c, 2, 44.0);
    equal(SIMD.Int32x4.extractLane(h, 2), 44.0);
    var i = SIMD.Int32x4.replaceLane(c, 3, 55.5); 
    equal(SIMD.Int32x4.extractLane(i, 3), 55.5);

    testSIMDInt32x4_conversion();
    testSIMDInt32x4_BinaryOp();
    testSIMDInt32x4_UnaryOp();
    testSIMDInt32x4_CompareOp();
    testSIMDInt32x4_Swizzle();
    testSIMDInt32x4_Shuffle();
    testSIMDInt32x4_Select();
}

function testSIMDInt32x4_conversion() {
    WScript.Echo("test SIMDInt32x4 Conversion......");
    var m = SIMD.Float32x4(1.0, 2.2, 3.6, 4.8);
    var n = SIMD.Int32x4.fromFloat32x4(m);
    equal(1, SIMD.Int32x4.extractLane(n, 0));
    equal(2, SIMD.Int32x4.extractLane(n, 1));
    equal(3, SIMD.Int32x4.extractLane(n, 2));
    equal(4, SIMD.Int32x4.extractLane(n, 3));

    var a = SIMD.Float64x2(1.0, 2.2);
    var b = SIMD.Int32x4.fromFloat64x2(a);
    equal(1, SIMD.Int32x4.extractLane(b, 0));
    equal(2, SIMD.Int32x4.extractLane(b, 1));
    equal(0, SIMD.Int32x4.extractLane(b, 2));
    equal(0, SIMD.Int32x4.extractLane(b, 3));

    var c = SIMD.Float32x4(1.0, 2.0, 3.0, 4.0);
    var d = SIMD.Int32x4.fromFloat32x4Bits(c);
    equal(0x3F800000, SIMD.Int32x4.extractLane(d, 0));
    equal(0x40000000, SIMD.Int32x4.extractLane(d, 1));
    equal(0x40400000, SIMD.Int32x4.extractLane(d, 2));
    equal(0x40800000, SIMD.Int32x4.extractLane(d, 3));

    var e = SIMD.Float64x2(1.0, 2.0);
    var f = SIMD.Int32x4.fromFloat64x2Bits(e);
    equal(0x00000000, SIMD.Int32x4.extractLane(f, 0));
    equal(0x3FF00000, SIMD.Int32x4.extractLane(f, 1));
    equal(0x00000000, SIMD.Int32x4.extractLane(f, 2));
    equal(0x40000000, SIMD.Int32x4.extractLane(f, 3));
}



function testSIMDInt32x4_BinaryOp() {
    WScript.Echo("test SIMDInt32x4 BinaryOp......");
    var m = SIMD.Int32x4(0xAAAAAAAA, 0xAAAAAAAA, -1431655766, 0xAAAAAAAA);
    var n = SIMD.Int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    equal(-1431655766, SIMD.Int32x4.extractLane(m, 0));
    equal(-1431655766, SIMD.Int32x4.extractLane(m, 1));
    equal(-1431655766, SIMD.Int32x4.extractLane(m, 2));
    equal(-1431655766, SIMD.Int32x4.extractLane(m, 3));
    equal(0x55555555, SIMD.Int32x4.extractLane(n, 0));
    equal(0x55555555, SIMD.Int32x4.extractLane(n, 1));
    equal(0x55555555, SIMD.Int32x4.extractLane(n, 2));
    equal(0x55555555, SIMD.Int32x4.extractLane(n, 3));
    var o = SIMD.Int32x4.and(m, n);  // and
    equal(0x0, SIMD.Int32x4.extractLane(o, 0));
    equal(0x0, SIMD.Int32x4.extractLane(o, 1));
    equal(0x0, SIMD.Int32x4.extractLane(o, 2));
    equal(0x0, SIMD.Int32x4.extractLane(o, 3));

    var m1 = SIMD.Int32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    var n1 = SIMD.Int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    var o1 = SIMD.Int32x4.or(m1, n1);  // or
    equal(-1, SIMD.Int32x4.extractLane(o1, 0));
    equal(-1, SIMD.Int32x4.extractLane(o1, 1));
    equal(-1, SIMD.Int32x4.extractLane(o1, 2));
    equal(-1, SIMD.Int32x4.extractLane(o1, 3));

    var m2 = SIMD.Int32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    var n2 = SIMD.Int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    n2 = SIMD.Int32x4.replaceLane(n2, 0, 0xAAAAAAAA);
    n2 = SIMD.Int32x4.replaceLane(n2, 1, 0xAAAAAAAA);
    n2 = SIMD.Int32x4.replaceLane(n2, 2, 0xAAAAAAAA);
    n2 = SIMD.Int32x4.replaceLane(n2, 3, 0xAAAAAAAA);
    equal(-1431655766, SIMD.Int32x4.extractLane(n2, 0));
    equal(-1431655766, SIMD.Int32x4.extractLane(n2, 1));
    equal(-1431655766, SIMD.Int32x4.extractLane(n2, 2));
    equal(-1431655766, SIMD.Int32x4.extractLane(n2, 3));
    var o2 = SIMD.Int32x4.xor(m2, n2);  // xor
    equal(0x0, SIMD.Int32x4.extractLane(o2, 0));
    equal(0x0, SIMD.Int32x4.extractLane(o2, 1));
    equal(0x0, SIMD.Int32x4.extractLane(o2, 2));
    equal(0x0, SIMD.Int32x4.extractLane(o2, 3));

    var a = SIMD.Int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x7fffffff, 0x0);
    var b = SIMD.Int32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c = SIMD.Int32x4.add(a, b);
    equal(0x0, SIMD.Int32x4.extractLane(c, 0));
    equal(-2, SIMD.Int32x4.extractLane(c, 1));
    equal(-0x80000000, SIMD.Int32x4.extractLane(c, 2));
    equal(-1, SIMD.Int32x4.extractLane(c, 3));

    var a1 = SIMD.Int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b1 = SIMD.Int32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c1 = SIMD.Int32x4.sub(a1, b1);
    equal(-2, SIMD.Int32x4.extractLane(c1, 0));
    equal(0x0, SIMD.Int32x4.extractLane(c1, 1));
    equal(0x7FFFFFFF, SIMD.Int32x4.extractLane(c1, 2));
    equal(0x1, SIMD.Int32x4.extractLane(c1, 3));

    var a2 = SIMD.Int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b2 = SIMD.Int32x4(0x1, 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF);
    var c2 = SIMD.Int32x4.mul(a2, b2);
    equal(-1, SIMD.Int32x4.extractLane(c2, 0));
    equal(0x1, SIMD.Int32x4.extractLane(c2, 1));
    equal(0x0, SIMD.Int32x4.extractLane(c2, 2));
    equal(0x0, SIMD.Int32x4.extractLane(c2, 3));
}

function testSIMDInt32x4_UnaryOp() {
    WScript.Echo("test SIMDInt32x4 UnaryOp......");
    var m = SIMD.Int32x4(16, 32, 64, 128);
    var n = SIMD.Int32x4(-1, -2, -3, -4);
    m = SIMD.Int32x4.neg(m);
    n = SIMD.Int32x4.neg(n);
    equal(-16, SIMD.Int32x4.extractLane(m, 0));
    equal(-32, SIMD.Int32x4.extractLane(m, 1));
    equal(-64, SIMD.Int32x4.extractLane(m, 2));
    equal(-128, SIMD.Int32x4.extractLane(m, 3));
    equal(1, SIMD.Int32x4.extractLane(n, 0));
    equal(2, SIMD.Int32x4.extractLane(n, 1));
    equal(3, SIMD.Int32x4.extractLane(n, 2));
    equal(4, SIMD.Int32x4.extractLane(n, 3));
}

function testSIMDInt32x4_CompareOp() {
    WScript.Echo("test SIMDInt32x4 CompareOp......");
    WScript.Echo("SIMDInt32x4 lessThan");
    var m = SIMD.Int32x4(1000, 2000, 100, 1);
    var n = SIMD.Int32x4(2000, 2000, 1, 100);
    var cmp;
    cmp = SIMD.Int32x4.lessThan(m, n);
    equal(true, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 3));

    WScript.Echo("SIMDInt32x4 equal");
    cmp = SIMD.Int32x4.equal(m, n);
    equal(false, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 3));

    WScript.Echo("SIMDInt32x4 greaterThan");
    cmp = SIMD.Int32x4.greaterThan(m, n);
    equal(false, SIMD.Bool32x4.extractLane(cmp, 0));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 1));
    equal(true, SIMD.Bool32x4.extractLane(cmp, 2));
    equal(false, SIMD.Bool32x4.extractLane(cmp, 3));

}

function testSIMDInt32x4_Swizzle() {
    WScript.Echo("test SIMDInt32x4 Swizzle......");
    var a = SIMD.Int32x4(1, 2, 3, 4);
    var xyxy = SIMD.Int32x4.swizzle(a, 0, 1, 0, 1);
    var zwzw = SIMD.Int32x4.swizzle(a, 2, 3, 2, 3);
    var xxxx = SIMD.Int32x4.swizzle(a, 0, 0, 0, 0);
    equal(1, SIMD.Int32x4.extractLane(xyxy, 0));
    equal(2, SIMD.Int32x4.extractLane(xyxy, 1));
    equal(1, SIMD.Int32x4.extractLane(xyxy, 2));
    equal(2, SIMD.Int32x4.extractLane(xyxy, 3));
    equal(3, SIMD.Int32x4.extractLane(zwzw, 0));
    equal(4, SIMD.Int32x4.extractLane(zwzw, 1));
    equal(3, SIMD.Int32x4.extractLane(zwzw, 2));
    equal(4, SIMD.Int32x4.extractLane(zwzw, 3));
    equal(1, SIMD.Int32x4.extractLane(xxxx, 0));
    equal(1, SIMD.Int32x4.extractLane(xxxx, 1));
    equal(1, SIMD.Int32x4.extractLane(xxxx, 2));
    equal(1, SIMD.Int32x4.extractLane(xxxx, 3));
}

function testSIMDInt32x4_Shuffle() {
    WScript.Echo("test SIMDInt32x4 Shuffle......");
    var a = SIMD.Int32x4(1, 2, 3, 4);
    var b = SIMD.Int32x4(5, 6, 7, 8);
    var xyxy = SIMD.Int32x4.shuffle(a, b, 0, 1, 4, 5);
    var zwzw = SIMD.Int32x4.shuffle(a, b, 2, 3, 6, 7);
    var xxxx = SIMD.Int32x4.shuffle(a, b, 0, 0, 4, 4);
    equal(1, SIMD.Int32x4.extractLane(xyxy, 0));
    equal(2, SIMD.Int32x4.extractLane(xyxy, 1));
    equal(5, SIMD.Int32x4.extractLane(xyxy, 2));
    equal(6, SIMD.Int32x4.extractLane(xyxy, 3));
    equal(3, SIMD.Int32x4.extractLane(zwzw, 0));
    equal(4, SIMD.Int32x4.extractLane(zwzw, 1));
    equal(7, SIMD.Int32x4.extractLane(zwzw, 2));
    equal(8, SIMD.Int32x4.extractLane(zwzw, 3));
    equal(1, SIMD.Int32x4.extractLane(xxxx, 0));
    equal(1, SIMD.Int32x4.extractLane(xxxx, 1));
    equal(5, SIMD.Int32x4.extractLane(xxxx, 2));
    equal(5, SIMD.Int32x4.extractLane(xxxx, 3));
}

function testSIMDInt32x4_Select() {
    WScript.Echo("test SIMDInt32x4 Select......");
    var m = SIMD.Bool32x4(true, true, false, false);
    var t = SIMD.Int32x4(1, 2, 3, 4);
    var f = SIMD.Int32x4(5, 6, 7, 8);
    var s = SIMD.Int32x4.select(m, t, f);
    equal(1, SIMD.Int32x4.extractLane(s, 0));
    equal(2, SIMD.Int32x4.extractLane(s, 1));
    equal(7, SIMD.Int32x4.extractLane(s, 2));
    equal(8, SIMD.Int32x4.extractLane(s, 3));
}

testSIMDInt32x4();

WScript.StartProfiling(testSIMDInt32x4);


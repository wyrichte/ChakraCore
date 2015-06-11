function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testConstructor() {
    WScript.Echo("Constructor");
    equal(SIMD.int32x4,  undefined);
    equal(SIMD.int32x4(1, 2, 3, 4),  undefined);
    var a = SIMD.int32x4("2014/10/10", -0, 2147483648, 2147483647);
    WScript.Echo("a.x: " + a.x);
    WScript.Echo("a.y: " + a.y);
    WScript.Echo("a.z: " + a.z);
    WScript.Echo("a.w: " + a.w);
    var b = SIMD.int32x4(4, 3, 2, 1);
    var c = SIMD.int32x4(b);
    equal(c, b);
    equal(c.x, b.x);
    equal(c.y, b.y);
    equal(c.z, b.z);
    equal(c.w, b.w);

    try {
        var m = SIMD.int32x4(1)
    }
    catch (e) {
        WScript.Echo("Type Error");
    }
}

function testFromFloat64x2(){
    var m = SIMD.float64x2(1.0, 2.2);
    var n = SIMD.int32x4.fromFloat64x2(m);
    WScript.Echo("FromFloat64x2");
    equal( 1, n.x);
    equal( 2, n.y);
    equal( 0, n.z);
    equal( 0, n.w);
}

function testFromFloat32x4()
{
    var m = SIMD.float32x4(1.0, 2.2, 3.6, 4.8);
    var n = SIMD.int32x4.fromFloat32x4(m);
    WScript.Echo("FromFloat32x4");
    equal(1, n.x);
    equal(2, n.y);
    equal(3, n.z);
    equal(4, n.w);
}

function testFromFloat64x2Bits()
{
    var m = SIMD.float64x2.fromInt32x4Bits(SIMD.int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000));
    var n = SIMD.int32x4.fromFloat64x2Bits(m);
    WScript.Echo("FromFloat64x2Bits");
    equal( 1, n.x);
    equal( 2, n.y);
    equal( 3, n.z);
    equal( 4, n.w);
    var a = SIMD.float64x2(1.0, 2.0);
    var b = SIMD.int32x4.fromFloat64x2Bits(a);
    equal(0x00000000, n.x);
    equal(0x3FF00000, n.y);
    equal(0x00000000, n.z);
    equal(0x40000000, n.w);
}

function testFromFloat32x4Bits()
{
    var m = SIMD.float32x4.fromInt32x4Bits(SIMD.int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000));
    var n = SIMD.int32x4.fromFloat32x4Bits(m);
    WScript.Echo("FromFloat32x4Bits");
    equal( 1, n.x);
    equal( 2, n.y);
    equal( 3, n.z);
    equal( 4, n.w);
    var a = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var b = SIMD.int32x4.fromFloat32x4Bits(a);
    equal(0x3F800000, b.x);
    equal(0x40000000, b.y);
    equal(0x40400000, b.z);
    equal(0x40800000, b.w);
}

testConstructor();
testFromFloat64x2();
testFromFloat32x4();
testFromFloat64x2Bits();
testFromFloat32x4Bits();


testConstructor();
testFromFloat64x2();
testFromFloat32x4();
testFromFloat64x2Bits();
testFromFloat32x4Bits();

testConstructor();
testFromFloat64x2();
testFromFloat32x4();
testFromFloat64x2Bits();
testFromFloat32x4Bits();

testConstructor();
testFromFloat64x2();
testFromFloat32x4();
testFromFloat64x2Bits();
testFromFloat32x4Bits();

testConstructor();
testFromFloat64x2();
testFromFloat32x4();
testFromFloat64x2Bits();
testFromFloat32x4Bits();

testConstructor();
testFromFloat64x2();
testFromFloat32x4();
testFromFloat64x2Bits();
testFromFloat32x4Bits();

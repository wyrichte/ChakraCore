function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testInt32x4BitConversion()
{
    WScript.Echo("float32x4 int32x4 bit conversion");
    var m = SIMD.int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000);
    var n = SIMD.float32x4.fromInt32x4Bits(m);
    equal(1.0, n.x);
    equal(2.0, n.y);
    equal(3.0, n.z);
    equal(4.0, n.w);
    n = SIMD.float32x4(5.0, 6.0, 7.0, 8.0);
    m = SIMD.int32x4.fromFloat32x4Bits(n);
    equal(0x40A00000, m.x);
    equal(0x40C00000, m.y);
    equal(0x40E00000, m.z);
    equal(0x41000000, m.w);
    // Flip sign using bit-wise operators.
    n = SIMD.float32x4(9.0, 10.0, 11.0, 12.0);
    m = SIMD.int32x4(0x80000000, 0x80000000, 0x80000000, 0x80000000);
    var nMask = SIMD.int32x4.fromFloat32x4Bits(n);
    nMask = SIMD.int32x4.xor(nMask, m); // flip sign.
    n = SIMD.float32x4.fromInt32x4Bits(nMask);
    equal(-9.0, n.x);
    equal(-10.0, n.y);
    equal(-11.0, n.z);
    equal(-12.0, n.w);
    nMask = SIMD.int32x4.fromFloat32x4Bits(n);
    nMask = SIMD.int32x4.xor(nMask, m); // flip sign.
    n = SIMD.float32x4.fromInt32x4Bits(nMask);
    equal(9.0, n.x);
    equal(10.0, n.y);
    equal(11.0, n.z);
    equal(12.0, n.w);
    // Should stay unmodified across bit conversions
    m = SIMD.int32x4(0xFFFFFFFF, 0xFFFF0000, 0x80000000, 0x0);
    var m2 = SIMD.int32x4.fromFloat32x4Bits(SIMD.float32x4.fromInt32x4Bits(m));
    equal(m.x, m2.x);
    equal(m.y, m2.y);
    equal(m.z, m2.z);
    equal(m.w, m2.w);
}

function testFloat64x2Conversion()
{
    WScript.Echo("int32x4 float64x2 conversion");
    var m = SIMD.int32x4(1, 2, 3, 4);
    var n = SIMD.float64x2.fromInt32x4(m);
    equal(1.0, n.x);
    equal(2.0, n.y);
    var a = SIMD.float64x2(1.0, 2.0);
    var b = SIMD.int32x4.fromFloat64x2(a);
    equal(1.0, b.x);
    equal(2.0, b.y);
}

function testFloat32x4Conversion() {
    WScript.Echo("int32x4 float32x4 conversion");
    var m = SIMD.int32x4(1, 2, 3, 4);
    var n = SIMD.float32x4.fromInt32x4(m);
    equal(1.0, n.x);
    equal(2.0, n.y);
    equal(3.0, n.z);
    equal(4.0, n.w);
    var a = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var b = SIMD.int32x4.fromFloat32x4(a);
    equal(1.0, b.x);
    equal(2.0, b.y);
    equal(3.0, b.z);
    equal(4.0, b.w);
}

testInt32x4BitConversion();
testInt32x4BitConversion();
testInt32x4BitConversion();
testInt32x4BitConversion();
testInt32x4BitConversion();
testInt32x4BitConversion();
testInt32x4BitConversion();

testFloat64x2Conversion();
testFloat64x2Conversion();
testFloat64x2Conversion();
testFloat64x2Conversion();
testFloat64x2Conversion();
testFloat64x2Conversion();
testFloat64x2Conversion();
testFloat64x2Conversion();

testFloat32x4Conversion();
testFloat32x4Conversion();
testFloat32x4Conversion();
testFloat32x4Conversion();
testFloat32x4Conversion();
testFloat32x4Conversion();
testFloat32x4Conversion();
testFloat32x4Conversion();

function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testConstructor() 
{
    WScript.Echo("Constructor");
    WScript.Echo(SIMD.float32x4 !== undefined);
    equal('function', typeof SIMD.float32x4);

    WScript.Echo(SIMD.float32x4(1.0, 2.0, 3.0, 4.0) !== undefined);

    var a = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var b = SIMD.float32x4(a);
    equal(a, b);
    try
    {
        var a = SIMD.float32x4(1)
    }
    catch(e)
    {
        WScript.Echo("Type Error");
    }
}

function  testFromFloat64x2()
{
    var m = SIMD.float64x2(1.0, 2.0);
    var n = SIMD.float32x4.fromFloat64x2(m);
    WScript.Echo("FromFloat64x2");
    equal( 1.0 , n.x);
    equal( 2.0 , n.y);
    equal( 0.0 , n.z);
    equal( 0.0 , n.w);
}

function  testFromInt32x4()
{
    var m = SIMD.int32x4(1, 2, 3, 4);
    var n = SIMD.float32x4.fromInt32x4(m);
    WScript.Echo("FromInt32x4");
    equal( 1.0 , n.x);
    equal( 2.0 , n.y);
    equal( 3.0 , n.z);
    equal( 4.0 , n.w);
}

function  testFromFloat64x2Bits()
{
    var m = SIMD.float64x2.fromInt32x4Bits(SIMD.int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000));
    var n = SIMD.float32x4.fromFloat64x2Bits(m);
    WScript.Echo("FromFloat64x2Bits");
    equal( 1.0 , n.x);
    equal( 2.0 , n.y);
    equal( 3.0 , n.z);
    equal( 4.0 , n.w);
}

function  testFromInt32x4Bits()
{
    var m = SIMD.int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000);
    var n = SIMD.float32x4.fromInt32x4Bits(m);
    WScript.Echo("FromInt32x4Bits");
    equal( 1.0 , n.x);
    equal( 2.0 , n.y);
    equal( 3.0 , n.z);
    equal( 4.0 , n.w);
}

testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();

testFromFloat64x2();
testFromFloat64x2();
testFromFloat64x2();
testFromFloat64x2();
testFromFloat64x2();
testFromFloat64x2();
testFromFloat64x2();
testFromFloat64x2();

testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();

testFromFloat64x2Bits();
testFromFloat64x2Bits();
testFromFloat64x2Bits();
testFromFloat64x2Bits();
testFromFloat64x2Bits();
testFromFloat64x2Bits();
testFromFloat64x2Bits();
testFromFloat64x2Bits();

testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();

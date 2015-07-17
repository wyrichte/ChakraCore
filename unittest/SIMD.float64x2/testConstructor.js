function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testConstructor() {
    WScript.Echo("Constructor");
    equal('function', typeof SIMD.float64x2);
    var m = SIMD.float64x2(1.0, 2.0);
    equal(1.0, m.x);
    equal(2.0, m.y);

    var a = SIMD.float64x2(1.0, 2.0);
    var b = SIMD.float64x2.check(a);
    equal(a, b);
    try
    {
        var a = SIMD.float64x2.check(1)
    }
    catch(e)
    {
        WScript.Echo("Type Error");
    }
}

function testSplatConstructor()
{
    WScript.Echo("Splat Constructor");
    equal('function', typeof SIMD.float64x2.splat);
    var m = SIMD.float64x2.splat(3.0);
    equal(3.0, m.x);
    equal(m.x, m.y);
}

function testZeroConstructor()
{
    equal('function', typeof SIMD.float64x2.zero);
    var m = SIMD.float64x2.zero();
    equal(0.0, m.x);
    equal(0.0, m.y);
}

function testFromFloat32x4 (){
    WScript.Echo("FromFloat32x4");
    var m = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var n = SIMD.float64x2.fromFloat32x4(m);
    equal(1.0, n.x);
    equal(2.0, n.y);
}


function testFromInt32x4 ()
{
    WScript.Echo("FromInt32x4");
    var m = SIMD.int32x4(1, 2, 3, 4);
    var n = SIMD.float64x2.fromInt32x4(m);
    equal(1.0, n.x);
    equal(2.0, n.y);
}

function testFromFloat32x4Bits()
{
    WScript.Echo("FromFloat32x4Bits");
    var m = SIMD.float32x4.fromInt32x4Bits(SIMD.int32x4(0x00000000, 0x3ff00000, 0x0000000, 0x40000000));
    var n = SIMD.float64x2.fromFloat32x4Bits(m);
    equal(1.0, n.x);
    equal(2.0, n.y);
}

function testFromInt32x4Bits ()
{
    WScript.Echo("FromInt32x4Bits");
    var m = SIMD.int32x4(0x00000000, 0x3ff00000, 0x0000000, 0x40000000);
    var n = SIMD.float64x2.fromInt32x4Bits(m);
    equal(1.0, n.x);
    equal(2.0, n.y);
}

testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();

testSplatConstructor();
testSplatConstructor();
testSplatConstructor();
testSplatConstructor();
testSplatConstructor();
testSplatConstructor();
testSplatConstructor();
testSplatConstructor();

testZeroConstructor();
testZeroConstructor();
testZeroConstructor();
testZeroConstructor();
testZeroConstructor();
testZeroConstructor();
testZeroConstructor();
testZeroConstructor();

testFromFloat32x4();
testFromFloat32x4();
testFromFloat32x4();
testFromFloat32x4();
testFromFloat32x4();
testFromFloat32x4();
testFromFloat32x4();
testFromFloat32x4();

testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();

testFromFloat32x4Bits();
testFromFloat32x4Bits();
testFromFloat32x4Bits();
testFromFloat32x4Bits();
testFromFloat32x4Bits();
testFromFloat32x4Bits();
testFromFloat32x4Bits();

testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();



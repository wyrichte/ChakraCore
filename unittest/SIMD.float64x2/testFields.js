function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct")
    else 
        WScript.Echo(">> Fail!")
}

function  testScalarGetters()
{
    WScript.Echo('float64x2 scalar getters');
    var m = SIMD.float64x2(1.0, 2.0);
    equal(1.0, m.x);
    equal(2.0, m.y);
}

function  testSignMask()
{
    WScript.Echo('float64x2 signMask');
    var a = SIMD.float64x2(1.0, 2.0);
    equal(0x0, a.signMask);
    var b = SIMD.float64x2(-1.0, 2.0);
    equal(0x1, b.signMask);
    var c = SIMD.float64x2(1.0, -2.0);
    equal(0x2, c.signMask);
    var d = SIMD.float64x2(-1.0, -2.0);
    equal(0x3, d.signMask);
    var e = SIMD.float64x2(0.0, -0.0);
    equal(0x2, e.signMask);
    var f = SIMD.float64x2(-0.0, 0.0);
    equal(0x1, f.signMask);
}


testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();

testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();

function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct")
    else 
        WScript.Echo(">> Fail!")
}

function  testScalarGetters()
{
    WScript.Echo('float32x4 scalar getters');
    var a = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    equal(1.0, a.x);
    equal(2.0, a.y);
    equal(3.0, a.z);
    equal(4.0, a.w);
}

function  testSignMask()
{
    WScript.Echo('float32x4 signMask');
    var a = SIMD.float32x4(-1.0, -2.0, -3.0, -4.0);
    equal(0xf, a.signMask);
    var b = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    equal(0x0, b.signMask);
    var c = SIMD.float32x4(1.0, -2.0, -3.0, 4.0);
    equal(0x6, c.signMask);
    var d = SIMD.float32x4(-0.0, 0.0, 0.0, -0.0);
    equal(0x9, d.signMask);
    var e = SIMD.float32x4(0.0, -0.0, -0.0, 0.0);
    equal(0x6, e.signMask);
}

function  testVectorGetters()
{
    WScript.Echo('float32x4 vector getters');
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var xxxx = SIMD.float32x4.shuffle(a, SIMD.XXXX);
    var yyyy = SIMD.float32x4.shuffle(a, SIMD.YYYY);
    var zzzz = SIMD.float32x4.shuffle(a, SIMD.ZZZZ);
    var wwww = SIMD.float32x4.shuffle(a, SIMD.WWWW);
    var wzyx = SIMD.float32x4.shuffle(a, SIMD.WZYX);
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
/*
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
testScalarGetters();
*/
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
/*
testVectorGetters();
testVectorGetters();
testVectorGetters();
testVectorGetters();
testVectorGetters();
testVectorGetters();
testVectorGetters();
*/
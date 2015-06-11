function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct")
    else 
        WScript.Echo(">> Fail!")
}

function  testScalarGetters()
{
    WScript.Echo('int32x4 scalar getters');
    var a = SIMD.int32x4(1, 2, 3, 4);
    equal(1, a.x);
    equal(2, a.y);
    equal(3, a.z);
    equal(4, a.w);
}

function  testSignMask()
{
    WScript.Echo('int32x4 signMask');
    var a = SIMD.int32x4(-1, -2, -3, -4);
    equal(0xf, a.signMask);
    var b = SIMD.int32x4(1, 2, 3, 4);
    equal(0x0, b.signMask);
    var c = SIMD.int32x4(1, -2, -3, 4);
    equal(0x6, c.signMask);
    var d = SIMD.int32x4(-0, 0, 0, -0);
    equal(0x0, d.signMask);
    var e = SIMD.int32x4(0, -0, -0, 0);
    equal(0x0, e.signMask);
}

function  testVectorGetters()
{
    WScript.Echo('int32x4 vector getters');
    var a = SIMD.int32x4(4, 3, 2, 1);
    var xxxx = SIMD.int32x4.shuffle(a, SIMD.XXXX);
    var yyyy = SIMD.int32x4.shuffle(a, SIMD.YYYY);
    var zzzz = SIMD.int32x4.shuffle(a, SIMD.ZZZZ);
    var wwww = SIMD.int32x4.shuffle(a, SIMD.WWWW);
    var wzyx = SIMD.int32x4.shuffle(a, SIMD.WZYX);
    equal(4, xxxx.x);
    equal(4, xxxx.y);
    equal(4, xxxx.z);
    equal(4, xxxx.w);
    equal(3, yyyy.x);
    equal(3, yyyy.y);
    equal(3, yyyy.z);
    equal(3, yyyy.w);
    equal(2, zzzz.x);
    equal(2, zzzz.y);
    equal(2, zzzz.z);
    equal(2, zzzz.w);
    equal(1, wwww.x);
    equal(1, wwww.y);
    equal(1, wwww.z);
    equal(1, wwww.w);
    equal(1, wzyx.x);
    equal(2, wzyx.y);
    equal(3, wzyx.z);
    equal(4, wzyx.w);
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

testVectorGetters();
testVectorGetters();
testVectorGetters();
testVectorGetters();
testVectorGetters();
testVectorGetters();
testVectorGetters();

function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testMul()
{
    WScript.Echo("int32x4 mul");
    var a = SIMD.int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b = SIMD.int32x4(0x1, 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF);
    var c = SIMD.int32x4.mul(a, b);
    equal(-1, c.x);
    equal(0x1, c.y);
    equal(0x0, c.z);
    equal(0x0, c.w);

    var d = SIMD.int32x4(4, 3, 2, 1);
    var e = SIMD.int32x4(10, 20, 30, 40);
    var f = SIMD.int32x4.mul(d, e);
    equal(40, f.x);
    equal(60, f.y);
    equal(60, f.z);
    equal(40, f.w);
}

testMul();
testMul();
testMul();
testMul();
testMul();
testMul();
testMul();
testMul();

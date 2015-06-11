function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testNeg()
{
    WScript.Echo("int32x4 neg");
    var a = SIMD.int32x4(-4, -3, -2, -1);
    var c = SIMD.int32x4.neg(a);
    equal(4, c.x);
    equal(3, c.y);
    equal(2, c.z);
    equal(1, c.w);
    c = SIMD.int32x4.neg(SIMD.int32x4(4, 3, 2, 1));
    equal(-4, c.x);
    equal(-3, c.y);
    equal(-2, c.z);
    equal(-1, c.w);
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

testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();

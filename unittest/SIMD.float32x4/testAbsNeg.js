function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testAbs()
{
    WScript.Echo("float32x4 abs");
    var a = SIMD.float32x4(-4.0, -3.0, -2.0, -1.0);
    var c = SIMD.float32x4.abs(a);
    equal(4.0, c.x);
    equal(3.0, c.y);
    equal(2.0, c.z);
    equal(1.0, c.w);
    c = SIMD.float32x4.abs(SIMD.float32x4(4.0, 3.0, 2.0, 1.0));
    equal(4.0, c.x);
    equal(3.0, c.y);
    equal(2.0, c.z);
    equal(1.0, c.w);
}

function testNeg()
{
    WScript.Echo("float32x4 neg");
    var a = SIMD.float32x4(-4.0, -3.0, -2.0, -1.0);
    var c = SIMD.float32x4.neg(a);
    equal(4.0, c.x);
    equal(3.0, c.y);
    equal(2.0, c.z);
    equal(1.0, c.w);
    c = SIMD.float32x4.neg(SIMD.float32x4(4.0, 3.0, 2.0, 1.0));
    equal(-4.0, c.x);
    equal(-3.0, c.y);
    equal(-2.0, c.z);
    equal(-1.0, c.w);
}

testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();

testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();



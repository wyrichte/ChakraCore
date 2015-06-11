function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testSqrt()
{
    WScript.Echo("float32x4 Sqrt");
    var a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    var c = SIMD.float32x4.sqrt(a);
    equal(4.0, c.x);
    equal(3.0, c.y);
    equal(2.0, c.z);
    equal(1.0, c.w);
}

testSqrt();
testSqrt();
testSqrt();
testSqrt();
testSqrt();
testSqrt();
testSqrt();

function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testSqrt()
{
    WScript.Echo("float64x2 Sqrt");
    var a = SIMD.float64x2(16.0, 9.0);
    var c = SIMD.float64x2.sqrt(a);
    equal(4.0, c.x);
    equal(3.0, c.y);
}

testSqrt();
testSqrt();
testSqrt();
testSqrt();
testSqrt();
testSqrt();
testSqrt();

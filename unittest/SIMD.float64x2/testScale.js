function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testScale()
{
    WScript.Echo("float64x2 Scale");
    var a = SIMD.float64x2(8.0, -2.0);
    var c = SIMD.float64x2.scale(a, 0.5);
    equal(4.0, c.x);
    equal(-1.0, c.y);
}

testScale();
testScale();
testScale();
testScale();
testScale();
testScale();
testScale();

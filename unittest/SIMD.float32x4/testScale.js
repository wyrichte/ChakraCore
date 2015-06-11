function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testScale()
{
    WScript.Echo("float32x4 Scale");
    var a = SIMD.float32x4(8.0, 4.0, 2.0, -2.0);
    var c = SIMD.float32x4.scale(a, 0.5);
    equal(4.0, c.x);
    equal(2.0, c.y);
    equal(1.0, c.z);
    equal(-1.0, c.w);
}

testScale();
testScale();
testScale();
testScale();
testScale();
testScale();
testScale();

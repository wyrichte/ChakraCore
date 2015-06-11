function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testClamp()
{
    WScript.Echo("float32x4 clamp");
    var a = SIMD.float32x4(-20.0, 10.0, 30.0, 0.5);
    var lower = SIMD.float32x4(2.0, 1.0, 50.0, 0.0);
    var upper = SIMD.float32x4(2.5, 5.0, 55.0, 1.0);
    var c = SIMD.float32x4.clamp(a, lower, upper);
    equal(2.0, c.x);
    equal(5.0, c.y);
    equal(50.0, c.z);
    equal(0.5, c.w);
}


testClamp();
testClamp();
testClamp();
testClamp();
testClamp();
testClamp();
testClamp();
testClamp();

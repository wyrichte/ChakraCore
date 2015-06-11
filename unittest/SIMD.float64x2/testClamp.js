function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testClamp()
{
    WScript.Echo("float64x2 clamp");
    var a = SIMD.float64x2(-20.0, 10.0);
    var b = SIMD.float64x2(2.125, 3.0);
    var lower = SIMD.float64x2(2.0, 1.0);
    var upper = SIMD.float64x2(2.5, 5.0);
    var c = SIMD.float64x2.clamp(a, lower, upper);
    equal(2.0, c.x);
    equal(5.0, c.y);
    c = SIMD.float64x2.clamp(b, lower, upper);
    equal(2.125, c.x);
    equal(3.0, c.y);
    a = SIMD.float64x2(-3.4e200, 3.4e250);
    b = SIMD.float64x2(3.4e100, 3.4e200);
    lower = SIMD.float64x2(3.4e50, 3.4e100);
    upper = SIMD.float64x2(3.4e150, 3.4e300);
    c = SIMD.float64x2.clamp(a, lower, upper);
    equal(3.4e50, c.x);
    equal(3.4e250, c.y);
    c = SIMD.float64x2.clamp(b, lower, upper);
    equal(3.4e100, c.x);
    equal(3.4e200, c.y);
}


testClamp();
testClamp();
testClamp();
testClamp();
testClamp();
testClamp();
testClamp();
testClamp();

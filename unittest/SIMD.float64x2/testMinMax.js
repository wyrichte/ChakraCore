function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testMin()
{
    WScript.Echo("float64x2 Min");
    var a = SIMD.float64x2(-20.0, 10.0);
    var lower = SIMD.float64x2(2.0, 1.0);
    var c = SIMD.float64x2.min(a, lower);
    equal(-20.0, c.x);
    equal(1.0, c.y);
}

function testMax()
{
    WScript.Echo("float64x2 Max");
    var a = SIMD.float64x2(-20.0, 10.0);
    var upper = SIMD.float64x2(2.5, 5.0);
    var c = SIMD.float64x2.max(a, upper);
    equal(2.5, c.x);
    equal(10.0, c.y);
}


testMin();
testMin();
testMin();
testMin();
testMin();
testMin();
testMin();

testMax();
testMax();
testMax();
testMax();
testMax();
testMax();
testMax();
testMax();

function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testMin()
{
    WScript.Echo("float32x4 Min");
    var a = SIMD.float32x4(-20.0, 10.0, 30.0, 0.5);
    var lower = SIMD.float32x4(2.0, 1.0, 50.0, 0.0);
    var c = SIMD.float32x4.min(a, lower);
    equal(-20.0, c.x);
    equal(1.0, c.y);
    equal(30.0, c.z);
    equal(0.0, c.w);
}

function testMax()
{
    WScript.Echo("float32x4 Max");
    var a = SIMD.float32x4(-20.0, 10.0, 30.0, 0.5);
    var upper = SIMD.float32x4(2.5, 5.0, 55.0, 1.0);
    var c = SIMD.float32x4.max(a, upper);
    equal(2.5, c.x);
    equal(10.0, c.y);
    equal(55.0, c.z);
    equal(1.0, c.w);
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

function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testMul()
{
    WScript.Echo("float32x4 mul");
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.float32x4.mul(a, b);
    equal(40.0, c.x);
    equal(60.0, c.y);
    equal(60.0, c.z);
    equal(40.0, c.w);
}

function testDiv()
{
    WScript.Echo("float32x4 div");
    var a = SIMD.float32x4(4.0, 9.0, 8.0, 1.0);
    var b = SIMD.float32x4(2.0, 3.0, 1.0, 0.5);
    var c = SIMD.float32x4.div(a, b);
    equal(2.0, c.x);
    equal(3.0, c.y);
    equal(8.0, c.z);
    equal(2.0, c.w);
}

testMul();
testMul();
testMul();
testMul();
testMul();
testMul();
testMul();
testMul();

testDiv();
testDiv();
testDiv();
testDiv();
testDiv();
testDiv();
testDiv();
testDiv();
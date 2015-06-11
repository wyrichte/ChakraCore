function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testMul()
{
    WScript.Echo("float64x2 mul");
    var a = SIMD.float64x2(2.0, 1.0);
    var b = SIMD.float64x2(10.0, 20.0);
    var c = SIMD.float64x2.mul(a, b);
    equal(20.0, c.x);
    equal(20.0, c.y);
}

function testDiv()
{
    WScript.Echo("float64x2 div");
    var a = SIMD.float64x2(4.0, 9.0);
    var b = SIMD.float64x2(2.0, 3.0);
    var c = SIMD.float64x2.div(a, b);
    equal(2.0, c.x);
    equal(3.0, c.y);
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
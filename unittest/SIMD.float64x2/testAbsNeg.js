function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testAbs()
{
    WScript.Echo("float64x2 abs");
    var a = SIMD.float64x2(-2.0, -1.0);
    var c = SIMD.float64x2.abs(a);
    equal(2.0, c.x);
    equal(1.0, c.y);
    c = SIMD.float64x2.abs(SIMD.float64x2(2.0, 1.0));
    equal(2.0, c.x);
    equal(1.0, c.y);
}

function testNeg()
{
    WScript.Echo("float64x2 neg");
    var a = SIMD.float64x2(-2.0, -1.0);
    var c = SIMD.float64x2.neg(a);
    equal(2.0, c.x);
    equal(1.0, c.y);
    c = SIMD.float64x2.neg(SIMD.float64x2(2.0, 1.0));
    equal(-2.0, c.x);
    equal(-1.0, c.y);
}

testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();

testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();


function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testNot()
{
    WScript.Echo("int32x4 not");
    var a = SIMD.int32x4(-4, -3, -2, -1);
    var c = SIMD.int32x4.not(a);
    equal(3, c.x);
    equal(2, c.y);
    equal(1, c.z);
    equal(0, c.w);
    c = SIMD.int32x4.not(SIMD.int32x4(4, 3, 2, 1));
    equal(-5, c.x);
    equal(-4, c.y);
    equal(-3, c.z);
    equal(-2, c.w);
}

testNot();
testNot();
testNot();
testNot();
testNot();
testNot();
testNot();
testNot();



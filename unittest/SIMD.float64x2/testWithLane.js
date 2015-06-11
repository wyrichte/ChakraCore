function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function equalNaN(a)
{
    if (isNaN(a))
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testWithX()
{
    WScript.Echo("float64x2 withX");
    var a = SIMD.float64x2(16.0, 9.0);
    var c = SIMD.float64x2.withX(a, 20.0);
    equal(20.0, c.x);
    equal(9.0, c.y);

    a = SIMD.float64x2(16.0, 9.0);
    c = SIMD.float64x2.withX(a);
    equalNaN(c.x);
    equal(9.0, c.y);
}

function testWithY()
{
    WScript.Echo("float64x2 withY");
    var a = SIMD.float64x2(16.0, 9.0);
    var c = SIMD.float64x2.withY(a, 20.0);
    equal(16.0, c.x);
    equal(20.0, c.y);

    a = SIMD.float64x2(16.0, 9.0);
    c = SIMD.float64x2.withX(a);
    equal(16.0, c.x);
    equalNaN(c.y);
}


testWithX();
testWithX();
testWithX();
testWithX();
testWithX();
testWithX();
testWithX();
testWithX();

testWithY();
testWithY();
testWithY();
testWithY();
testWithY();
testWithY();
testWithY();
testWithY();

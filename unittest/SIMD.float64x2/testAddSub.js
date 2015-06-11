function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testAdd()
{
    WScript.Echo("float64x2 add");
    var a = SIMD.float64x2(2.0, 1.0);
    var b = SIMD.float64x2(10.0, 20.0);
    var c = SIMD.float64x2.add(a, b);
    equal(12.0, c.x);
    equal(21.0, c.y);
}

function testSub()
{
    WScript.Echo("float64x2 sub");
    var a = SIMD.float64x2(2.0, 1.0);
    var b = SIMD.float64x2(10.0, 20.0);
    var c = SIMD.float64x2.sub(a, b);
    equal(-8.0, c.x);
    equal(-19.0, c.y);
}

testAdd();
testAdd();
testAdd();
testAdd();
testAdd();
testAdd();
testAdd();

testSub();
testSub();
testSub();
testSub();
testSub();
testSub();
testSub();


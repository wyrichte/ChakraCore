function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testAdd()
{
    WScript.Echo("float32x4 add");
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.float32x4.add(a, b);
    equal(14.0, c.x);
    equal(23.0, c.y);
    equal(32.0, c.z);
    equal(41.0, c.w);
}

function testSub()
{
    WScript.Echo("float32x4 sub");
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.float32x4.sub(a, b);
    equal(-6.0, c.x);
    equal(-17.0, c.y);
    equal(-28.0, c.z);
    equal(-39.0, c.w);
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


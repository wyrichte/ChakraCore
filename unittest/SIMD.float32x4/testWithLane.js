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
    WScript.Echo("float32x4 withX");
    var a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    var c = SIMD.float32x4.withX(a, 20.0);
    equal(20.0, c.x);
    equal(9.0, c.y);
    equal(4.0, c.z);
    equal(1.0, c.w);

    a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    c = SIMD.float32x4.withX(a);
    equalNaN(c.x);
    equal(9.0, c.y);
    equal(4.0, c.z);
    equal(1.0, c.w);
}

function testWithY()
{
    WScript.Echo("float32x4 withY");
    var a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    var c = SIMD.float32x4.withY(a, 20.0);
    equal(16.0, c.x);
    equal(20.0, c.y);
    equal(4.0, c.z);
    equal(1.0, c.w);

    a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    c = SIMD.float32x4.withY(a);
    equal(16.0, c.x);
    equalNaN(c.y);
    equal(4.0, c.z);
    equal(1.0, c.w);
}

function testWithZ()
{
    WScript.Echo("float32x4 withZ");
    var a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    var c = SIMD.float32x4.withZ(a, 20.0);
    equal(16.0, c.x);
    equal(9.0, c.y);
    equal(20.0, c.z);
    equal(1.0, c.w);  

    a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    c = SIMD.float32x4.withZ(a);
    equal(16.0, c.x);
    equal(9.0, c.y);
    equalNaN(c.z);
    equal(1.0, c.w);
}

function testWithW()
{
    WScript.Echo("float32x4 withW");
    var a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    var c = SIMD.float32x4.withW(a, 20.0);
    equal(16.0, c.x);
    equal(9.0, c.y);
    equal(4.0, c.z);
    equal(20.0, c.w);

    a = SIMD.float32x4(16.0, 9.0, 4.0, 1.0);
    c = SIMD.float32x4.withW(a);
    equal(16.0, c.x);
    equal(9.0, c.y);
    equal(4.0, c.z);
    equalNaN(c.w);
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

testWithZ();
testWithZ();
testWithZ();
testWithZ();
testWithZ();
testWithZ();
testWithZ();
testWithZ();

testWithW();
testWithW();
testWithW();
testWithW();
testWithW();
testWithW();
testWithW();
testWithW();
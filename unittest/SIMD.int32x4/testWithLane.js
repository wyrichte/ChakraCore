function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testWithX()
{
    WScript.Echo("int32x4 withX");
    var a = SIMD.int32x4(16, 9, 4, 1);
    var c = SIMD.int32x4.withX(a, 20);
    equal(20, c.x);
    equal(9, c.y);
    equal(4, c.z);
    equal(1, c.w);
    var m = SIMD.int32x4(1, 2, 3, 4);
    var n = SIMD.int32x4.withX(m, 20);
    equal(20, n.x);
    equal(2, n.y);
    equal(3, n.z);
    equal(4, n.w);
	
    m = SIMD.int32x4(1, 2, 3, 4);
    n = SIMD.int32x4.withX(m);
    equal(0, n.x);
    equal(2, n.y);
    equal(3, n.z);
    equal(4, n.w);
}

function testWithY()
{
    WScript.Echo("int32x4 withY");
    var a = SIMD.int32x4(16, 9, 4, 1);
    var c = SIMD.int32x4.withY(a, 20);
    equal(16, c.x);
    equal(20, c.y);
    equal(4, c.z);
    equal(1, c.w);
    var m = SIMD.int32x4(1, 2, 3, 4);
    var n = SIMD.int32x4.withY(m, 20);
    equal(1, n.x);
    equal(20, n.y);
    equal(3, n.z);
    equal(4, n.w);
	
    m = SIMD.int32x4(1, 2, 3, 4);
    n = SIMD.int32x4.withY(m);
    equal(1, n.x);
    equal(0, n.y);
    equal(3, n.z);
    equal(4, n.w);
}

function testWithZ()
{
    WScript.Echo("int32x4 withZ");
    var a = SIMD.int32x4(16, 9, 4, 1);
    var c = SIMD.int32x4.withZ(a, 20);
    equal(16, c.x);
    equal(9, c.y);
    equal(20, c.z);
    equal(1, c.w);
    var m = SIMD.int32x4(1, 2, 3, 4);
    var n = SIMD.int32x4.withZ(m, 20);
    equal(1, n.x);
    equal(2, n.y);
    equal(20, n.z);
    equal(4, n.w);
	
    m = SIMD.int32x4(1, 2, 3, 4);
    n = SIMD.int32x4.withZ(m);
    equal(1, n.x);
    equal(2, n.y);
    equal(0, n.z);
    equal(4, n.w);
}

function testWithW()
{
    WScript.Echo("int32x4 withW");
    var a = SIMD.int32x4(16, 9, 4, 1);
    var c = SIMD.int32x4.withW(a, 20);
    equal(16, c.x);
    equal(9, c.y);
    equal(4, c.z);
    equal(20, c.w);
    var m = SIMD.int32x4(1, 2, 3, 4);
    var n = SIMD.int32x4.withW(m, 20);
    equal(1, n.x);
    equal(2, n.y);
    equal(3, n.z);
    equal(20, n.w);
	
    m = SIMD.int32x4(1, 2, 3, 4);
    n = SIMD.int32x4.withW(m);
    equal(1, n.x);
    equal(2, n.y);
    equal(3, n.z);
    equal(0, n.w);
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

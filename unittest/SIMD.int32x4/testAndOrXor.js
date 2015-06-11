function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testAnd()
{
    WScript.Echo("int32x4 and");
    var m = SIMD.int32x4(0xAAAAAAAA, 0xAAAAAAAA, -1431655766, 0xAAAAAAAA);
    var n = SIMD.int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    equal(-1431655766, m.x);
    equal(-1431655766, m.y);
    equal(-1431655766, m.z);
    equal(-1431655766, m.w);
    equal(0x55555555, n.x);
    equal(0x55555555, n.y);
    equal(0x55555555, n.z);
    equal(0x55555555, n.w);
    equal(true, n.flagX);
    equal(true, n.flagY);
    equal(true, n.flagZ);
    equal(true, n.flagW);
    var o = SIMD.int32x4.and(m, n);  // and
    equal(0x0, o.x);
    equal(0x0, o.y);
    equal(0x0, o.z);
    equal(0x0, o.w);
    equal(false, o.flagX);
    equal(false, o.flagY);
    equal(false, o.flagZ);
    equal(false, o.flagW);
}

function testOr()
{
    WScript.Echo("int32x4 or");
    var m = SIMD.int32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    var n = SIMD.int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    var o = SIMD.int32x4.or(m, n);  // or
    equal(-1, o.x);
    equal(-1, o.y);
    equal(-1, o.z);
    equal(-1, o.w);
    equal(true, o.flagX);
    equal(true, o.flagY);
    equal(true, o.flagZ);
    equal(true, o.flagW);
}

function testXor()
{
    var m = SIMD.int32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    var n = SIMD.int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    n = SIMD.int32x4.withX(n, 0xAAAAAAAA);
    n = SIMD.int32x4.withY(n, 0xAAAAAAAA);
    n = SIMD.int32x4.withZ(n, 0xAAAAAAAA);
    n = SIMD.int32x4.withW(n, 0xAAAAAAAA);
    equal(-1431655766, n.x);
    equal(-1431655766, n.y);
    equal(-1431655766, n.z);
    equal(-1431655766, n.w);
    var o = SIMD.int32x4.xor(m, n);  // xor
    equal(0x0, o.x);
    equal(0x0, o.y);
    equal(0x0, o.z);
    equal(0x0, o.w);
    equal(false, o.flagX);
    equal(false, o.flagY);
    equal(false, o.flagZ);
    equal(false, o.flagW);
}

testAnd();
testAnd();
testAnd();
testAnd();

testOr();
testOr();
testOr();
testOr();

testXor();
testXor();
testXor();
testXor();

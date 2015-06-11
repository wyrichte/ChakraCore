function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testAdd()
{
    WScript.Echo("int32x4 add");
    var a = SIMD.int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x7fffffff, 0x0);
    var b = SIMD.int32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c = SIMD.int32x4.add(a, b);
    equal(0x0, c.x);
    equal(-2, c.y);
    equal(-0x80000000, c.z);
    equal(-1, c.w);

    var m = SIMD.int32x4(4, 3, 2, 1);
    var n = SIMD.int32x4(10, 20, 30, 40);
    var f = SIMD.int32x4.add(m, n);
    equal(14, f.x);
    equal(23, f.y);
    equal(32, f.z);
    equal(41, f.w);
}

function testSub()
{
    WScript.Echo("int32x4 sub");
    var a = SIMD.int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b = SIMD.int32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c = SIMD.int32x4.sub(a, b);
    equal(-2, c.x);
    equal(0x0, c.y);
    equal(0x7FFFFFFF, c.z);
    equal(0x1, c.w);

    var d = SIMD.int32x4(4, 3, 2, 1);
    var e = SIMD.int32x4(10, 20, 30, 40);
    var f = SIMD.int32x4.sub(d, e);
    equal(-6, f.x);
    equal(-17, f.y);
    equal(-28, f.z);
    equal(-39, f.w);
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

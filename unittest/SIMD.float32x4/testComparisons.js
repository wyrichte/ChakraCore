function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testComparisons()
{
    WScript.Echo("float32x4 lessThan");
    var m = SIMD.float32x4(1.0, 2.0, 0.1, 0.001);
    var n = SIMD.float32x4(2.0, 2.0, 0.001, 0.1);
    var cmp;
    cmp = SIMD.float32x4.lessThan(m, n);
    equal(-1, cmp.x);
    equal(0x0, cmp.y);
    equal(0x0, cmp.z);
    equal(-1, cmp.w);  

    WScript.Echo("float32x4 lessThanOrEqual");
    cmp = SIMD.float32x4.lessThanOrEqual(m, n);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(-1, cmp.w);

    WScript.Echo("float32x4 equal");
    cmp = SIMD.float32x4.equal(m, n);
    equal(0x0, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
	
    WScript.Echo("float32x4 notEqual");
    cmp = SIMD.float32x4.notEqual(m, n);
    equal(-1, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);

    WScript.Echo("float32x4 greaterThanOrEqual");
    cmp = SIMD.float32x4.greaterThanOrEqual(m, n);
    equal(0x0, cmp.x);
    equal(-1, cmp.y);
    equal(-1, cmp.z);
    equal(0x0, cmp.w);

    WScript.Echo("float32x4 greaterThan");
    cmp = SIMD.float32x4.greaterThan(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(0x0, cmp.w);
}


testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();

function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testComparisons()
{
    WScript.Echo("float64x2 lessThan");
    var m = SIMD.float64x2(1.0, 2.0);
    var n = SIMD.float64x2(2.0, 2.0);
    var o = SIMD.float64x2(0.1, 0.001);
    var p = SIMD.float64x2(0.001, 0.1);

    var cmp;
    cmp = SIMD.float64x2.lessThan(m, n);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
    cmp = SIMD.float64x2.lessThan(o, p);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);

    WScript.Echo("float64x2 lessThanOrEqual");
    cmp = SIMD.float64x2.lessThanOrEqual(m, n);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);
    cmp = SIMD.float64x2.lessThanOrEqual(o, p);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);

    WScript.Echo("float64x2 equal");
    cmp = SIMD.float64x2.equal(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);
    cmp = SIMD.float64x2.equal(o, p);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);

    WScript.Echo("float64x2 notEqual");
    cmp = SIMD.float64x2.notEqual(m, n);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
    cmp = SIMD.float64x2.notEqual(o, p);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);

    WScript.Echo("float64x2 greaterThanOrEqual");
    cmp = SIMD.float64x2.greaterThanOrEqual(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(-1, cmp.z);
    equal(-1, cmp.w);
    cmp = SIMD.float64x2.greaterThanOrEqual(o, p);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);

    WScript.Echo("float64x2 greaterThan");
    cmp = SIMD.float64x2.greaterThan(m, n);
    equal(0x0, cmp.x);
    equal(0x0, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
    cmp = SIMD.float64x2.greaterThan(o, p);
    equal(-1, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
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

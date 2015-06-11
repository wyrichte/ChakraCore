function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testComparisons()
{
    WScript.Echo("int32x4 lessThan");
    var m = SIMD.int32x4(1000, 2000, 100, 1);
    var n = SIMD.int32x4(2000, 2000,   1, 100);
    var cmp;
    cmp = SIMD.int32x4.lessThan(m, n);
    equal(-1, cmp.x);
    equal(0x0, cmp.y);
    equal(0x0, cmp.z);
    equal(-1, cmp.w);  

    WScript.Echo("int32x4 equal");
    cmp = SIMD.int32x4.equal(m, n);
    equal(0x0, cmp.x);
    equal(-1, cmp.y);
    equal(0x0, cmp.z);
    equal(0x0, cmp.w);
	
    WScript.Echo("int32x4 greaterThan");
    cmp = SIMD.int32x4.greaterThan(m, n);
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

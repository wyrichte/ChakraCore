function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testComparisons() {
    WScript.Echo("Int32x4 lessThan");
    var m = SIMD.Int32x4(1000, 2000, 100, 1);
    var n = SIMD.Int32x4(2000, 2000, 1, 100);
    var cmp;
    cmp = SIMD.Int32x4.lessThan(m, n);
    equal(-1, SIMD.Int32x4.extractLane(cmp, 0));
    equal(0x0, SIMD.Int32x4.extractLane(cmp, 1));
    equal(0x0, SIMD.Int32x4.extractLane(cmp, 2));
    equal(-1, SIMD.Int32x4.extractLane(cmp, 3));

    WScript.Echo("Int32x4 equal");
    cmp = SIMD.Int32x4.equal(m, n);
    equal(0x0, SIMD.Int32x4.extractLane(cmp, 0));
    equal(-1, SIMD.Int32x4.extractLane(cmp, 1));
    equal(0x0, SIMD.Int32x4.extractLane(cmp, 2));
    equal(0x0, SIMD.Int32x4.extractLane(cmp, 3));

    WScript.Echo("Int32x4 greaterThan");
    cmp = SIMD.Int32x4.greaterThan(m, n);
    equal(0x0, SIMD.Int32x4.extractLane(cmp, 0));
    equal(0x0, SIMD.Int32x4.extractLane(cmp, 1));
    equal(-1, SIMD.Int32x4.extractLane(cmp, 2));
    equal(0x0, SIMD.Int32x4.extractLane(cmp, 3));
}


testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();

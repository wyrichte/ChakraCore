function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testComparisons() {
    WScript.Echo("Int8x16 lessThan");
    var m = SIMD.Int8x16(50, 100, 10, 1, 50, 100, 10, 1, 50, 100, 10, 1, 50, 100, 10, 1);
    var n = SIMD.Int8x16(100, 100, 1, 10, 100, 100, 1, 10, 100, 100, 1, 10, 100, 100, 1, 10);
    var cmp;
    cmp = SIMD.Int8x16.lessThan(m, n);

    equal(-1, SIMD.Int8x16.extractLane(cmp, 0));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 1));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 2));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 3));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 4));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 5));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 6));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 7));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 8));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 9));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 10));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 11));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 12));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 13));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 14));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 15));

    WScript.Echo("Int8x16 equal");
    cmp = SIMD.Int8x16.equal(m, n);
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 0));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 1));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 2));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 3));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 4));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 5));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 6));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 7));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 8));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 9));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 10));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 11));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 12));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 13));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 14));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 15));

    WScript.Echo("Int8x16 greaterThan");
    cmp = SIMD.Int8x16.greaterThan(m, n);
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 0));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 1));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 2));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 3));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 4));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 5));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 6));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 7));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 8));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 9));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 10));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 11));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 12));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 13));
    equal(-1, SIMD.Int8x16.extractLane(cmp, 14));
    equal(0x0, SIMD.Int8x16.extractLane(cmp, 15));

}

testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();
testComparisons();

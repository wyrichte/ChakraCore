function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testNeg() {
    WScript.Echo("Int32x4 neg");
    var a = SIMD.Int32x4(-4, -3, -2, -1);
    var c = SIMD.Int32x4.neg(a);
    equal(4, SIMD.Int32x4.extractLane(c, 0));
    equal(3, SIMD.Int32x4.extractLane(c, 1));
    equal(2, SIMD.Int32x4.extractLane(c, 2));
    equal(1, SIMD.Int32x4.extractLane(c, 3));
    c = SIMD.Int32x4.neg(SIMD.Int32x4(4, 3, 2, 1));
    equal(-4, SIMD.Int32x4.extractLane(c, 0));
    equal(-3, SIMD.Int32x4.extractLane(c, 1));
    equal(-2, SIMD.Int32x4.extractLane(c, 2));
    equal(-1, SIMD.Int32x4.extractLane(c, 3));
    var m = SIMD.Int32x4(16, 32, 64, 128);
    var n = SIMD.Int32x4(-1, -2, -3, -4);
    m = SIMD.Int32x4.neg(m);
    n = SIMD.Int32x4.neg(n);
    equal(-16, SIMD.Int32x4.extractLane(m, 0));
    equal(-32, SIMD.Int32x4.extractLane(m, 1));
    equal(-64, SIMD.Int32x4.extractLane(m, 2));
    equal(-128, SIMD.Int32x4.extractLane(m, 3));
    equal(1, SIMD.Int32x4.extractLane(n, 0));
    equal(2, SIMD.Int32x4.extractLane(n, 1));
    equal(3, SIMD.Int32x4.extractLane(n, 2));
    equal(4, SIMD.Int32x4.extractLane(n, 3));
}

testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();

function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testAbs() {
    WScript.Echo("Float32x4 abs");
    var a = SIMD.Float32x4(-4.0, -3.0, -2.0, -1.0);
    var c = SIMD.Float32x4.abs(a);
    equal(4.0, SIMD.Float32x4.extractLane(c, 0));
    equal(3.0, SIMD.Float32x4.extractLane(c, 1));
    equal(2.0, SIMD.Float32x4.extractLane(c, 2));
    equal(1.0, SIMD.Float32x4.extractLane(c, 3));
    c = SIMD.Float32x4.abs(SIMD.Float32x4(4.0, 3.0, 2.0, 1.0));
    equal(4.0, SIMD.Float32x4.extractLane(c, 0));
    equal(3.0, SIMD.Float32x4.extractLane(c, 1));
    equal(2.0, SIMD.Float32x4.extractLane(c, 2));
    equal(1.0, SIMD.Float32x4.extractLane(c, 3));
}

function testNeg() {
    WScript.Echo("Float32x4 neg");
    var a = SIMD.Float32x4(-4.0, -3.0, -2.0, -1.0);
    var c = SIMD.Float32x4.neg(a);
    equal(4.0, SIMD.Float32x4.extractLane(c, 0));
    equal(3.0, SIMD.Float32x4.extractLane(c, 1));
    equal(2.0, SIMD.Float32x4.extractLane(c, 2));
    equal(1.0, SIMD.Float32x4.extractLane(c, 3));
    c = SIMD.Float32x4.neg(SIMD.Float32x4(4.0, 3.0, 2.0, 1.0));
    equal(-4.0, SIMD.Float32x4.extractLane(c, 0));
    equal(-3.0, SIMD.Float32x4.extractLane(c, 1));
    equal(-2.0, SIMD.Float32x4.extractLane(c, 2));
    equal(-1.0, SIMD.Float32x4.extractLane(c, 3));
}

testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();
testAbs();

testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();
testNeg();



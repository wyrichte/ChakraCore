function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testMin() {
    WScript.Echo("Float32x4 Min");
    var a = SIMD.Float32x4(-20.0, 10.0, 30.0, 0.5);
    var lower = SIMD.Float32x4(2.0, 1.0, 50.0, 0.0);
    var c = SIMD.Float32x4.min(a, lower);
    equal(-20.0, SIMD.Float32x4.extractLane(c, 0));
    equal(1.0, SIMD.Float32x4.extractLane(c, 1));
    equal(30.0, SIMD.Float32x4.extractLane(c, 2));
    equal(0.0, SIMD.Float32x4.extractLane(c, 3));
}

function testMax() {
    WScript.Echo("Float32x4 Max");
    var a = SIMD.Float32x4(-20.0, 10.0, 30.0, 0.5);
    var upper = SIMD.Float32x4(2.5, 5.0, 55.0, 1.0);
    var c = SIMD.Float32x4.max(a, upper);
    equal(2.5, SIMD.Float32x4.extractLane(c, 0));
    equal(10.0, SIMD.Float32x4.extractLane(c, 1));
    equal(55.0, SIMD.Float32x4.extractLane(c, 2));
    equal(1.0, SIMD.Float32x4.extractLane(c, 3));
}


testMin();
testMin();
testMin();
testMin();
testMin();
testMin();
testMin();

testMax();
testMax();
testMax();
testMax();
testMax();
testMax();
testMax();
testMax();

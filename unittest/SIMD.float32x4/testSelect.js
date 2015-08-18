function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testSelect() {
    WScript.Echo("Float32x4 Select");
    var m = SIMD.Int32x4.bool(true, true, false, false);
    var t = SIMD.Float32x4(1.0, 2.0, 3.0, 4.0);
    var f = SIMD.Float32x4(5.0, 6.0, 7.0, 8.0);
    var s = SIMD.Float32x4.select(m, t, f);
    equal(1.0, SIMD.Float32x4.extractLane(s, 0));
    equal(2.0, SIMD.Float32x4.extractLane(s, 1));
    equal(7.0, SIMD.Float32x4.extractLane(s, 2));
    equal(8.0, SIMD.Float32x4.extractLane(s, 3));
}

testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();

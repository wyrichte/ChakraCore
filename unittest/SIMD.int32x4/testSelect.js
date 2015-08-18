function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testSelect() {
    WScript.Echo("Int32x4 Select");
    var m = SIMD.Int32x4.bool(true, true, false, false);
    var t = SIMD.Int32x4(1, 2, 3, 4);
    var f = SIMD.Int32x4(5, 6, 7, 8);
    var s = SIMD.Int32x4.select(m, t, f);
    equal(1, SIMD.Int32x4.extractLane(s, 0));
    equal(2, SIMD.Int32x4.extractLane(s, 1));
    equal(7, SIMD.Int32x4.extractLane(s, 2));
    equal(8, SIMD.Int32x4.extractLane(s, 3));
}

testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();

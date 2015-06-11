function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testSelect()
{
    WScript.Echo("float64x2 Select");
    var m = SIMD.int32x4.bool(true, true, false, false);
    var t = SIMD.float64x2(1.0, 2.0);
    var f = SIMD.float64x2(3.0, 4.0);
    var s = SIMD.float64x2.select(m, t, f);
    equal(1.0, s.x);
    equal(4.0, s.y);
}

testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();

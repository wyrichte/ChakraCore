function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testSelect()
{
    WScript.Echo("float32x4 Select");
    var m = SIMD.int32x4.bool(true, true, false, false);
    var t = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var f = SIMD.float32x4(5.0, 6.0, 7.0, 8.0);
    var s = SIMD.float32x4.select(m, t, f);
    equal(1.0, s.x);
    equal(2.0, s.y);
    equal(7.0, s.z);
    equal(8.0, s.w);
}

testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();

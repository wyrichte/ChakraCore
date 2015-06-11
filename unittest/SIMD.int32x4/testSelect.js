function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testSelect()
{
    WScript.Echo("int32x4 Select");
    var m = SIMD.int32x4.bool(true, true, false, false);
    var t = SIMD.int32x4(1, 2, 3, 4);
    var f = SIMD.int32x4(5, 6, 7, 8);
    var s = SIMD.int32x4.select(m, t, f);
    equal(1, s.x);
    equal(2, s.y);
    equal(7, s.z);
    equal(8, s.w);
}

testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();
testSelect();

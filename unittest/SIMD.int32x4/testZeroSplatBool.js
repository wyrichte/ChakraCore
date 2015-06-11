function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testZero() {
    WScript.Echo("zero");
    var z = SIMD.int32x4.zero();
    equal(0, z.x);
    equal(0, z.y);
    equal(0, z.z);
    equal(0, z.w);
}

function testSplat(){
    var n = SIMD.int32x4.splat(3);
    WScript.Echo("splat");
    equal(3, n.x);
    equal(3, n.y);
    equal(3, n.z);
    equal(3, n.w);
}

function testBool()
{
    var n = SIMD.int32x4.bool(true, false, true, false);
    WScript.Echo("bool");
    equal(-1,  n.x);
    equal(0,   n.y);
    equal(-1,  n.z);
    equal(0,   n.w);
}

testZero();
testZero();
testZero();
testZero();
testZero();
testZero();
testZero();
testZero();

testSplat();
testSplat();
testSplat();
testSplat();
testSplat();
testSplat();
testSplat();
testSplat();

testBool();
testBool();
testBool();
testBool();
testBool();
testBool();
testBool();
testBool();


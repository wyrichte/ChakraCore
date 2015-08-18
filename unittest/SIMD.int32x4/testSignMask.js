function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testSignMask() {
    WScript.Echo("Int32x4 signmask");
    var a = SIMD.Int32x4(0x80000000, 0x7000000, 0xFFFFFFFF, 0x0);
    equal(0x5, a.signMask);
    var b = SIMD.Int32x4(0x0, 0x0, 0x0, 0x0);
    equal(0x0, b.signMask);
    var c = SIMD.Int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);
    equal(0xf, c.signMask);
}

testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();
testSignMask();



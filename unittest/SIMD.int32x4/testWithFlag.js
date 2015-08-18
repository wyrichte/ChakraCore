function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testWithFlagX() {
    WScript.Echo("Int32x4 withFlagX");
    var a = SIMD.Int32x4.bool(true, false, true, false);
    var c = SIMD.Int32x4.withFlagX(a, true);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    c = SIMD.Int32x4.withFlagX(a, false);
    equal(false, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    equal(0x0, SIMD.Int32x4.extractLane(c, 0));
    equal(0x0, SIMD.Int32x4.extractLane(c, 1));
    equal(-1, SIMD.Int32x4.extractLane(c, 2));
    equal(0x0, SIMD.Int32x4.extractLane(c, 3));
}

function testWithFlagY() {
    WScript.Echo("Int32x4 withFlagY");
    var a = SIMD.Int32x4.bool(true, false, true, false);
    var c = SIMD.Int32x4.withFlagY(a, true);
    equal(true, c.flagX);
    equal(true, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    c = SIMD.Int32x4.withFlagY(a, false);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    equal(-1, SIMD.Int32x4.extractLane(c, 0));
    equal(0x0, SIMD.Int32x4.extractLane(c, 1));
    equal(-1, SIMD.Int32x4.extractLane(c, 2));
    equal(0x0, SIMD.Int32x4.extractLane(c, 3));
}

function testWithFlagZ() {
    WScript.Echo("Int32x4 withFlagZ");
    var a = SIMD.Int32x4.bool(true, false, true, false);
    var c = SIMD.Int32x4.withFlagZ(a, true);
    equal(-1, SIMD.Int32x4.extractLane(c, 0));
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    c = SIMD.Int32x4.withFlagZ(a, false);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(false, c.flagZ);
    equal(false, c.flagW);
    equal(-1, SIMD.Int32x4.extractLane(c, 0));
    equal(0x0, SIMD.Int32x4.extractLane(c, 1));
    equal(0x0, SIMD.Int32x4.extractLane(c, 1));
    equal(0x0, SIMD.Int32x4.extractLane(c, 3));
}

function testWithFlagW() {
    WScript.Echo("Int32x4 withFlagW");
    var a = SIMD.Int32x4.bool(true, false, true, false);
    var c = SIMD.Int32x4.withFlagW(a, true);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(true, c.flagW);
    c = SIMD.Int32x4.withFlagW(a, false);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    equal(-1, SIMD.Int32x4.extractLane(c, 0));
    equal(0x0, SIMD.Int32x4.extractLane(c, 1));
    equal(-1, SIMD.Int32x4.extractLane(c, 2));
    equal(0x0, SIMD.Int32x4.extractLane(c, 3));
}

testWithFlagX();
testWithFlagX();
testWithFlagX();
testWithFlagX();
testWithFlagX();
testWithFlagX();
testWithFlagX();
testWithFlagX();

testWithFlagY();
testWithFlagY();
testWithFlagY();
testWithFlagY();
testWithFlagY();
testWithFlagY();
testWithFlagY();
testWithFlagY();

testWithFlagZ();
testWithFlagZ();
testWithFlagZ();
testWithFlagZ();
testWithFlagZ();
testWithFlagZ();
testWithFlagZ();
testWithFlagZ();

testWithFlagW();
testWithFlagW();
testWithFlagW();
testWithFlagW();
testWithFlagW();
testWithFlagW();
testWithFlagW();
testWithFlagW();

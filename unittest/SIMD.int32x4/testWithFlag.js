function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testWithFlagX()
{
    WScript.Echo("int32x4 withFlagX");
    var a = SIMD.int32x4.bool(true, false, true, false);
    var c = SIMD.int32x4.withFlagX(a, true);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    c = SIMD.int32x4.withFlagX(a, false);
    equal(false, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    equal(0x0, c.x);
    equal(0x0, c.y);
    equal(-1,  c.z);
    equal(0x0, c.w);
}

function testWithFlagY()
{
    WScript.Echo("int32x4 withFlagY");
    var a = SIMD.int32x4.bool(true, false, true, false);
    var c = SIMD.int32x4.withFlagY(a, true);
    equal(true, c.flagX);
    equal(true, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    c = SIMD.int32x4.withFlagY(a, false);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    equal(-1,  c.x);
    equal(0x0, c.y);
    equal(-1,  c.z);
    equal(0x0, c.w);
}

function testWithFlagZ()
{
    WScript.Echo("int32x4 withFlagZ");
    var a = SIMD.int32x4.bool(true, false, true, false);
    var c = SIMD.int32x4.withFlagZ(a, true);
    equal(-1, c.x);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    c = SIMD.int32x4.withFlagZ(a, false);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(false, c.flagZ);
    equal(false, c.flagW);
    equal(-1, c.x);
    equal(0x0, c.y);
    equal(0x0, c.y);
    equal(0x0, c.w);
}

function testWithFlagW()
{
    WScript.Echo("int32x4 withFlagW");
    var a = SIMD.int32x4.bool(true, false, true, false);
    var c = SIMD.int32x4.withFlagW(a, true);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(true, c.flagW);
    c = SIMD.int32x4.withFlagW(a, false);
    equal(true, c.flagX);
    equal(false, c.flagY);
    equal(true, c.flagZ);
    equal(false, c.flagW);
    equal(-1, c.x);
    equal(0x0, c.y);
    equal(-1, c.z);
    equal(0x0, c.w);
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

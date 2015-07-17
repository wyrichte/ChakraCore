function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testSwizzle()
{
    WScript.Echo("float32x4 Shuffle");
    var a    = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var xyxy = SIMD.float32x4.swizzle(a, 0, 1, 0, 1);
    var zwzw = SIMD.float32x4.swizzle(a, 2, 3, 2, 3);
    var xxxx = SIMD.float32x4.swizzle(a, 0, 0, 0, 0);
    equal(1.0, xyxy.x);
    equal(2.0, xyxy.y);
    equal(1.0, xyxy.z);
    equal(2.0, xyxy.w);
    equal(3.0, zwzw.x);
    equal(4.0, zwzw.y);
    equal(3.0, zwzw.z);
    equal(4.0, zwzw.w);
    equal(1.0, xxxx.x);
    equal(1.0, xxxx.y);
    equal(1.0, xxxx.z);
    equal(1.0, xxxx.w);
}

function testShuffle()
{
    WScript.Echo("float32x4 ShuffleMix");
    var a    = SIMD.float32x4(1.0, 2.0, 3.0, 4.0);
    var b    = SIMD.float32x4(5.0, 6.0, 7.0, 8.0);
    var xyxy = SIMD.float32x4.shuffle(a, b, 0, 1, 4, 5);
    var zwzw = SIMD.float32x4.shuffle(a, b, 2, 3, 6, 7);
    var xxxx = SIMD.float32x4.shuffle(a, b, 0, 0, 4, 4);
    equal(1.0, xyxy.x);
    equal(2.0, xyxy.y);
    equal(5.0, xyxy.z);
    equal(6.0, xyxy.w);
    equal(3.0, zwzw.x);
    equal(4.0, zwzw.y);
    equal(7.0, zwzw.z);
    equal(8.0, zwzw.w);
    equal(1.0, xxxx.x);
    equal(1.0, xxxx.y);
    equal(5.0, xxxx.z);
    equal(5.0, xxxx.w);
}

testSwizzle();
testSwizzle();
testSwizzle();
testSwizzle();
testSwizzle();
testSwizzle();
testSwizzle();
testSwizzle();

testShuffle();
testShuffle();
testShuffle();
testShuffle();
testShuffle();
testShuffle();
testShuffle();
testShuffle();


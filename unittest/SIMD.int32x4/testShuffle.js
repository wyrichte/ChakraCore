function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testSwizzle()
{
    WScript.Echo("int32x4 Shuffle");
    var a    = SIMD.int32x4(1, 2, 3, 4);
    var xyxy = SIMD.int32x4.swizzle(a, 0, 1, 0, 1);
    var zwzw = SIMD.int32x4.swizzle(a, 2, 3, 2, 3);
    var xxxx = SIMD.int32x4.swizzle(a, 0, 0, 0, 0);
    equal(1, xyxy.x);
    equal(2, xyxy.y);
    equal(1, xyxy.z);
    equal(2, xyxy.w);
    equal(3, zwzw.x);
    equal(4, zwzw.y);
    equal(3, zwzw.z);
    equal(4, zwzw.w);
    equal(1, xxxx.x);
    equal(1, xxxx.y);
    equal(1, xxxx.z);
    equal(1, xxxx.w);
}

function testShuffle()
{
    WScript.Echo("int32x4 ShuffleMix");
    var a    = SIMD.int32x4(1, 2, 3, 4);
    var b    = SIMD.int32x4(5, 6, 7, 8);
    var xyxy = SIMD.int32x4.shuffle(a, b, 0, 1, 4, 5);
    var zwzw = SIMD.int32x4.shuffle(a, b, 2, 3, 6, 7);
    var xxxx = SIMD.int32x4.shuffle(a, b, 0, 0, 4, 4);
    equal(1, xyxy.x);
    equal(2, xyxy.y);
    equal(5, xyxy.z);
    equal(6, xyxy.w);
    equal(3, zwzw.x);
    equal(4, zwzw.y);
    equal(7, zwzw.z);
    equal(8, zwzw.w);
    equal(1, xxxx.x);
    equal(1, xxxx.y);
    equal(5, xxxx.z);
    equal(5, xxxx.w);
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


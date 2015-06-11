function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testShuffle()
{
    WScript.Echo("int32x4 Shuffle");
    var a    = SIMD.int32x4(1, 2, 3, 4);
    var xyxy = SIMD.int32x4.shuffle(a, SIMD.XYXY);
    var zwzw = SIMD.int32x4.shuffle(a, SIMD.ZWZW);
    var xxxx = SIMD.int32x4.shuffle(a, SIMD.XXXX);
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

function testShuffleMix()
{
    WScript.Echo("int32x4 ShuffleMix");
    var a    = SIMD.int32x4(1, 2, 3, 4);
    var b    = SIMD.int32x4(5, 6, 7, 8);
    var xyxy = SIMD.int32x4.shuffleMix(a, b, SIMD.XYXY);
    var zwzw = SIMD.int32x4.shuffleMix(a, b, SIMD.ZWZW);
    var xxxx = SIMD.int32x4.shuffleMix(a, b, SIMD.XXXX);
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

testShuffle();
testShuffle();
testShuffle();
testShuffle();
testShuffle();
testShuffle();
testShuffle();
testShuffle();

testShuffleMix();
testShuffleMix();
testShuffleMix();
testShuffleMix();
testShuffleMix();
testShuffleMix();
testShuffleMix();
testShuffleMix();


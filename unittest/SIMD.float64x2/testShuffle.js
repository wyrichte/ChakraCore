function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testSwizzle()
{
    WScript.Echo("float64x2 Shuffle");
    var a  = SIMD.float64x2(1.0, 2.0);
    var xx = SIMD.float64x2.swizzle(a, 0, 0);
    var xy = SIMD.float64x2.swizzle(a, 0, 1);
    var yx = SIMD.float64x2.swizzle(a, 1, 0);
    var yy = SIMD.float64x2.swizzle(a, 1, 1);
    equal(1.0, xx.x);
    equal(1.0, xx.y);
    equal(1.0, xy.x);
    equal(2.0, xy.y);
    equal(2.0, yx.x);
    equal(1.0, yx.y);
    equal(2.0, yy.x);
    equal(2.0, yy.y);
}

function testShuffle()
{
    WScript.Echo("float64x2 ShuffleMix");
    var a  = SIMD.float64x2(1.0, 2.0);
    var b  = SIMD.float64x2(3.0, 4.0);
    var xx = SIMD.float64x2.shuffle(a, b, 0, 2);
    var xy = SIMD.float64x2.shuffle(a, b, 0, 3);
    var yx = SIMD.float64x2.shuffle(a, b, 1, 2);
    var yy = SIMD.float64x2.shuffle(a, b, 1, 3);
    equal(1.0, xx.x);
    equal(3.0, xx.y);
    equal(1.0, xy.x);
    equal(4.0, xy.y);
    equal(2.0, yx.x);
    equal(3.0, yx.y);
    equal(2.0, yy.x);
    equal(4.0, yy.y);
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


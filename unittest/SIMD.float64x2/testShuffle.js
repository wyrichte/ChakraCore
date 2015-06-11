function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testShuffle()
{
    WScript.Echo("float64x2 Shuffle");
    var a  = SIMD.float64x2(1.0, 2.0);
    var xx = SIMD.float64x2.shuffle(a, SIMD.XX);
    var xy = SIMD.float64x2.shuffle(a, SIMD.XY);
    var yx = SIMD.float64x2.shuffle(a, SIMD.YX);
    var yy = SIMD.float64x2.shuffle(a, SIMD.YY);
    equal(1.0, xx.x);
    equal(1.0, xx.y);
    equal(1.0, xy.x);
    equal(2.0, xy.y);
    equal(2.0, yx.x);
    equal(1.0, yx.y);
    equal(2.0, yy.x);
    equal(2.0, yy.y);
}

function testShuffleMix()
{
    WScript.Echo("float64x2 ShuffleMix");
    var a  = SIMD.float64x2(1.0, 2.0);
    var b  = SIMD.float64x2(3.0, 4.0);
    var xx = SIMD.float64x2.shuffleMix(a, b, SIMD.XX);
    var xy = SIMD.float64x2.shuffleMix(a, b, SIMD.XY);
    var yx = SIMD.float64x2.shuffleMix(a, b, SIMD.YX);
    var yy = SIMD.float64x2.shuffleMix(a, b, SIMD.YY);
    equal(1.0, xx.x);
    equal(3.0, xx.y);
    equal(1.0, xy.x);
    equal(4.0, xy.y);
    equal(2.0, yx.x);
    equal(3.0, yx.y);
    equal(2.0, yy.x);
    equal(4.0, yy.y);
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


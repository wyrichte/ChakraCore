function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function testShiftleft()
{
    WScript.Echo("int32x4 shiftLeft");
    var a = SIMD.int32x4(0x80000000, 0x7000000, 0xFFFFFFFF, 0x0);
    var b = SIMD.int32x4.shiftLeft(a, 1)
    equal(0x0, b.x);
    equal(0xE000000, b.y);
    equal(-2, b.z);
    equal(0x0, b.w);
    var c = SIMD.int32x4(1, 2, 3, 4);
    var d = SIMD.int32x4.shiftLeft(c, 1)
    equal(2, d.x);
    equal(4, d.y);
    equal(6, d.z);
    equal(8, d.w);
}

function testShiftRightLogical() {
    WScript.Echo("int32x4 shiftRightLogical");
    var a = SIMD.int32x4(0x80000000, 0x7000000, 0xFFFFFFFF, 0x0);
    var b = SIMD.int32x4.shiftRightLogical(a, 1)
    equal(0x40000000, b.x);
    equal(0x03800000, b.y);
    equal(0x7FFFFFFF, b.z);
    equal(0x0, b.w);
    var c = SIMD.int32x4(1, 2, 3, 4);
    var d = SIMD.int32x4.shiftRightLogical(c, 1)
    equal(0, d.x);
    equal(1, d.y);
    equal(1, d.z);
    equal(2, d.w);
}

function testShiftRightArithmetic() {
    WScript.Echo("int32x4 shiftRightArithmetic");
    var a = SIMD.int32x4(0x80000000, 0x7000000, 0xFFFFFFFF, 0x0);
    var b = SIMD.int32x4.shiftRightArithmetic(a, 1)
    equal(-1073741824, b.x);
    //equal(0xC0000000, b.x);
    equal(0x03800000, b.y);
    equal(-1, b.z);
    equal(0x0, b.w);
    var c = SIMD.int32x4(1, 2, 3, 4);
    var d = SIMD.int32x4.shiftRightArithmetic(c, 1)
    equal(0, d.x);
    equal(1, d.y);
    equal(1, d.z);
    equal(2, d.w);
    var c = SIMD.int32x4(-1, -2, -3, -4);
    var d = SIMD.int32x4.shiftRightArithmetic(c, 1)
    equal(-1, d.x);
    equal(-1, d.y);
    equal(-2, d.z);
    equal(-2, d.w);
}

testShiftleft();
testShiftleft();
testShiftleft();
testShiftleft();

testShiftRightLogical();
testShiftRightLogical();
testShiftRightLogical();
testShiftRightLogical();

testShiftRightArithmetic();
testShiftRightArithmetic();
testShiftRightArithmetic();
testShiftRightArithmetic();

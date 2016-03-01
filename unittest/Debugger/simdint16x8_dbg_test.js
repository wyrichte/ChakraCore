//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

function testSIMDInt16x8_unaryop() {
    // test SIMDInt16x8 UnaryOp
    var m = SIMD.Int16x8(16, 32, 64, 28, 1, 2, 3, 4);
    var n = SIMD.Int16x8(-1, -2, -3, -4, -5, -6, -7, -8);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE)**/

    var p = SIMD.Int16x8.neg(m);    // neg
    p; /**bp:evaluate('p', 0, LOCALS_TYPE), evaluate('p',1)**/

    var q = SIMD.Int16x8.neg(n);    // neg
    q; /**bp:evaluate('q', 0, LOCALS_TYPE), evaluate('q',1)**/

    var a = SIMD.Int16x8(8, 7, 6, 5, 4, 3, 2, 1);
    var c = SIMD.Int16x8.not(a);    // not
    c; /**bp:evaluate('c', 0, LOCALS_TYPE), locals(1)**/

    var d = SIMD.Int16x8.not(SIMD.Int16x8(8, 7, 6, 5, 4, 3, 2, 1));
    d; /**bp:evaluate('d', 0, LOCALS_TYPE), locals(1)**/
}

function testSIMDInt16x8_binaryop() {
    // test SIMDInt32x4 BinaryOp
    var m = SIMD.Int16x8(0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA);
    var n = SIMD.Int16x8(0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE)**/

    var o = SIMD.Int16x8.and(m, n);  // and
    o; /**bp:locals(1);evaluate('o', 0, LOCALS_TYPE)**/

    var o1 = SIMD.Int16x8.or(m, n);  // or
    o1; /**bp:evaluate('o1', 2);evaluate('o1', 0, LOCALS_TYPE)**/

    var o2 = SIMD.Int16x8.xor(m, n);  // xor
    o2; /**bp:evaluate('o2', 2);**/

    var a = SIMD.Int16x8(4, 3, 2, 1, 5, 6, 7, 8);
    var b = SIMD.Int16x8(10, 20, 30, 40, 10, 20, 30, 40);
    var c = SIMD.Int16x8.add(a, b);
    c; /**bp:evaluate('c', 2);**/

    var d = SIMD.Int16x8(10, 20, 30, 40, 10, 20, 30, 40);
    var e = SIMD.Int16x8(4,   3,  2,  1,  4,  3,  2, 1);
    var f = SIMD.Int16x8.sub(d, e);
    f; /**bp:evaluate('f', 2);**/

    var d2 = SIMD.Int16x8(1, 2, 3, 4, 5, 6, 7, 8);
    var e2 = SIMD.Int16x8(8, 7, 6, 5, 4, 3, 2, 1);
    var f2 = SIMD.Int16x8.mul(d2, e2);
    f2; /**bp:evaluate('f2', 2);**/
}

function testSIMDInt16x8_compareop() {
    // test SIMDInt16x8 compareOp
    var m = SIMD.Int16x8(1, 2, 3, 4, 5, 6, 7, 8);
    m; /**bp:locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Int16x8(8, 7, 6, 5, 4, 3, 2, 1);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n', 2);**/
    var cmp = SIMD.Int16x8.lessThan(m, n);  // lessThan
    cmp; /**bp:evaluate('cmp', 0, LOCALS_TYPE), evaluate('cmp', 2);**/

    cmp = SIMD.Int16x8.equal(m, n);         // equal
    cmp; /**bp:evaluate('cmp', 2);**/

    cmp = SIMD.Int16x8.greaterThan(m, n);   // greaterThan
    cmp; /**bp:evaluate('cmp', 2);**/
}

function testSIMDInt16x8_conversion() {
    // test SIMDInt16x8 Conversion......
    a = SIMD.Int32x4(1, 2, 3, 4);
    /**bp:locals();stack()**/
    var b = SIMD.Int16x8.fromInt32x4Bits(a);
    /**bp:locals(1);bp:evaluate('b', 2);**/

    var f = SIMD.Int32x4.fromInt16x8Bits(b);
    /**bp:evaluate('f', 2);**/

    var c = SIMD.Int8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    /**bp:evaluate('c', 2);**/
    var d = SIMD.Int16x8.fromInt8x16Bits(c);
    /**bp:evaluate('d', 2);**/

    var e = SIMD.Int8x16.fromInt16x8Bits(d);
    /**bp:evaluate('e', 2);**/
}

function testSIMDInt16x8_replaceLane() {
    // test SIMDInt16x8.....
    var c = SIMD.Int16x8(1, 2, 3, 4, 5, 6, 7, 8);
    /**bp:locals();stack()**/

    // test SIMDInt16x8 WithLane......
    var f = SIMD.Int16x8.replaceLane(c, 0, 111);
    /**bp:evaluate('f', 1);**/

    var g = SIMD.Int16x8.replaceLane(c, 1, 222);
    /**bp:evaluate('g', 2);**/

    var h = SIMD.Int16x8.replaceLane(c, 2, 333);
    /**bp:evaluate('h', 2);resume('step_into');stack()**/

    var i = SIMD.Int16x8.replaceLane(c, 3, 444);
    /**bp:locals();bp:evaluate('i', 1);**/
}

testSIMDInt16x8_unaryop();
testSIMDInt16x8_binaryop();
testSIMDInt16x8_compareop();
testSIMDInt16x8_conversion();
testSIMDInt16x8_replaceLane();

WScript.Echo("PASS");
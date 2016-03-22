//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

function testSIMDInt8x16_unaryop() {
    // test SIMDInt8x16 UnaryOp......
    var m = SIMD.Int8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    var n = SIMD.Int8x16(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE)**/

    var p = SIMD.Int8x16.neg(m);    // neg
    p; /**bp:evaluate('p', 0, LOCALS_TYPE), evaluate('p',1)**/

    var a = SIMD.Int8x16(1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4);
    a; /**bp:evaluate('a', 0, LOCALS_TYPE)**/
    var c = SIMD.Int8x16.not(a);    // not
    c; /**bp:evaluate('c', 0, LOCALS_TYPE), locals(1)**/

    var d = SIMD.Int8x16.not(SIMD.Int8x16(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1));
    d; /**bp:evaluate('d', 0, LOCALS_TYPE)**/
}

function testSIMDInt8x16_binaryop() {
    // test SIMDInt32x4 BinaryOp
    var m = SIMD.Int8x16(0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA);
    var n = SIMD.Int8x16(0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE)**/

    var o = SIMD.Int8x16.and(m, n);  // and
    o; /**bp:locals(1);bp:evaluate('o', 0, LOCALS_TYPE)**/

    var o1 = SIMD.Int8x16.or(m, n);  // or
    o1; /**bp:bp:evaluate('o1', 0, LOCALS_TYPE), evaluate('o1', 2);**/

    var o2 = SIMD.Int8x16.xor(m, n);  // xor
    o2; /**bp:evaluate('o2', 2);**/

    var a = SIMD.Int8x16(4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1);
    var b = SIMD.Int8x16(10, 20, 30, 40, 10, 20, 30, 40, 10, 20, 30, 40, 10, 20, 30, 40);
    var c = SIMD.Int8x16.add(a, b);
    c; /**bp:evaluate('c', 2);**/

    var d = SIMD.Int8x16(10, 20, 30, 40, 10, 20, 30, 40, 10, 20, 30, 40, 10, 20, 30, 40);
    var e = SIMD.Int8x16(4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1);
    var f = SIMD.Int8x16.sub(d, e);
    f; /**bp:evaluate('f', 2);**/

    var d2 = SIMD.Int8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    var e2 = SIMD.Int8x16(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    var f2 = SIMD.Int8x16.mul(d2, e2);
    f2; /**bp:evaluate('f2', 2);**/
}

function testSIMDInt8x16_compareop() {
    // test SIMDInt8x16 compareOp
    var m = SIMD.Int8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    m; /**bp:locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Int8x16(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    n; /**bp:evaluate('n', 2);**/
    var cmp = SIMD.Int8x16.lessThan(m, n);  // lessThan
    cmp; /**bp:evaluate('cmp', 2);**/

    cmp = SIMD.Int8x16.equal(m, n);         // equal
    cmp; /**bp:evaluate('cmp', 2);**/

    cmp = SIMD.Int8x16.greaterThan(m, n);   // greaterThan
    cmp; /**bp:evaluate('cmp', 2);**/
}

function testSIMDInt8x16_conversion() {
    // test SIMDInt8x16 Conversion
    a = SIMD.Int32x4(1, 2, 3, 4);
    /**bp:locals();bp:evaluate('a', 0, LOCALS_TYPE)**/
    var b = SIMD.Int8x16.fromInt32x4Bits(a);
    b; /**bp:locals(1);bp:evaluate('b', 2);bp:evaluate('b', 0, LOCALS_TYPE)**/
}

function testSIMDInt8x16_replaceLane() {
    // test SIMDInt8x16
    var c = SIMD.Int8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    c; /**bp:locals();bp:evaluate('c', 0, LOCALS_TYPE)**/

    // test SIMDInt8x16 replaceLane
    var f = SIMD.Int8x16.replaceLane(c, 0, 111);
    f; /**bp:evaluate('f', 1);**/

    var g = SIMD.Int8x16.replaceLane(c, 1, 22);
    g; /**bp:evaluate('g', 2);**/

    var h = SIMD.Int8x16.replaceLane(c, 2, 33);
    h; /**bp:evaluate('h', 2);**/

    var i = SIMD.Int8x16.replaceLane(c, 3, 44);
    i; /**bp:locals();bp:evaluate('i', 1);**/
}

testSIMDInt8x16_unaryop();
testSIMDInt8x16_binaryop();
testSIMDInt8x16_compareop();
testSIMDInt8x16_conversion();
testSIMDInt8x16_replaceLane();

WScript.Echo("PASS");
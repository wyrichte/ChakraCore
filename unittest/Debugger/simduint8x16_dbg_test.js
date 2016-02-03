//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

function testSIMDUint8x16_unaryop() {
    // test SIMDUint8x16 UnaryOp
    var m = SIMD.Uint8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    var n = SIMD.Uint8x16(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE);**/

    var p = SIMD.Uint8x16.not(m);    // not
    p; /**bp:evaluate('p', 0, LOCALS_TYPE), evaluate('p',1)**/

    var a = SIMD.Uint8x16(1, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4);
    var c = SIMD.Uint8x16.not(a);    // not
    c; /**bp:evaluate('c', 0, LOCALS_TYPE); locals(1)**/

    var d = SIMD.Uint8x16.not(c);
    d; /**bp:evaluate('d',1)**/
}

function testSIMDUint8x16_binaryop() {
    // test SIMDUint8x16 BinaryOp
    var m = SIMD.Uint8x16(0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA);
    var n = SIMD.Uint8x16(0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE);**/

    var o = SIMD.Uint8x16.and(m, n);  // and
    o; /**bp:locals(1);evaluate('o', 0, LOCALS_TYPE)**/

    var o1 = SIMD.Uint8x16.or(m, n);  // or
    o1; /**bp:evaluate('o1', 2);evaluate('o1', 0, LOCALS_TYPE)**/

    var o2 = SIMD.Uint8x16.xor(m, n);  // xor
    o2; /**bp:evaluate('o2', 2);**/

    var a = SIMD.Uint8x16(4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1);
    var b = SIMD.Uint8x16(10, 20, 30, 40, 10, 20, 30, 40, 10, 20, 30, 40, 10, 20, 30, 40);
    var c = SIMD.Uint8x16.add(a, b);
    c; /**bp:evaluate('c', 2);**/

    var d = SIMD.Uint8x16(10, 20, 30, 40, 10, 20, 30, 40, 10, 20, 30, 40, 10, 20, 30, 40);
    var e = SIMD.Uint8x16(4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1, 4, 3, 2, 1);
    var f = SIMD.Uint8x16.sub(d, e);
    f; /**bp:evaluate('f', 2);**/

    var d2 = SIMD.Uint8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    var e2 = SIMD.Uint8x16(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    var f2 = SIMD.Uint8x16.mul(d2, e2);
    f2; /**bp:evaluate('f2', 2);**/
}

function testSIMDUint8x16_minmaxop() {
    // test SIMDUint8x16 minmaxOp
    var m = SIMD.Uint8x16(100, 200, 100, 1, 10, 20, 30, 40, 100, 200, 300, 400, 1, 2, 3, 4);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE);locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Uint8x16(200, 300, 1, 200, 400, 10, 22, 3, 1, 2, 3, 4, 100, 200, 300, 400);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE); evaluate('n', 2);**/
    var cmp = SIMD.Uint8x16.min(m, n);  // min
    cmp; /**bp:evaluate('cmp', 2);**/

    var cmp2 = SIMD.Uint8x16.max(m, n);      // max
    cmp2; /**bp:evaluate('cmp2', 2);**/
}

function testSIMDUint8x16_compareop() {
    // test SIMDUint8x16 compareOp
    var m = SIMD.Uint8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE); locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Uint8x16(16, 15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1);
    n; /**bp:evaluate('n', 2);**/
    var cmp = SIMD.Uint8x16.lessThan(m, n);  // lessThan
    cmp; /**bp:evaluate('cmp', 2);**/

    cmp = SIMD.Uint8x16.equal(m, n);         // equal
    cmp; /**bp:evaluate('cmp', 2);**/

    cmp = SIMD.Uint8x16.greaterThan(m, n);   // greaterThan
    cmp; /**bp:evaluate('cmp', 2);**/
}

function testSIMDUint8x16_conversion() {
    // test SIMDUint8x16 Conversion
    var a = SIMD.Int32x4(1, 2, 3, 4);
    a; /**bp:locals();evaluate('a', 0, LOCALS_TYPE);**/
    var b = SIMD.Uint8x16.fromInt32x4Bits(a);
    b; /**bp:locals(1);bp:evaluate('b', 2);evaluate('b', 0, LOCALS_TYPE);**/
    var c = SIMD.Int16x8(11, 22, 33, 44, 55, 66, 77, 88);
    c; /**bp:evaluate('c', 2);evaluate('c', 0, LOCALS_TYPE);**/
    var d = SIMD.Uint8x16.fromInt16x8Bits(c);
    d; /**bp:evaluate('d', 2);evaluate('d', 0, LOCALS_TYPE);**/
}

function testSIMDUint8x16_replaceLane() {
    // test SIMDUint8x16
    var c = SIMD.Uint8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    c; /**bp:locals();evaluate('c', 0, LOCALS_TYPE);**/

    // test SIMDUint8x16 WithLane
    var f = SIMD.Uint8x16.replaceLane(c, 0, 111);
    f; /**bp:evaluate('f', 0, LOCALS_TYPE); evaluate('f', 1);**/

    var g = SIMD.Uint8x16.replaceLane(c, 1, 22);
    g; /**bp:evaluate('g', 2);**/

    var h = SIMD.Uint8x16.replaceLane(c, 2, 33);
    h; /**bp:evaluate('h', 2);evaluate('h', 0, LOCALS_TYPE);**/

    var i = SIMD.Uint8x16.replaceLane(c, 3, 44);
    i; /**bp:locals();bp:evaluate('i', 1);**/

    var j = SIMD.Uint8x16.replaceLane(c, 4, 55);
    j; /**bp:evaluate('j', 1);**/

    var k = SIMD.Uint8x16.replaceLane(c, 5, 66);
    k; /**bp:evaluate('k', 1);**/

    var l = SIMD.Uint8x16.replaceLane(c, 6, 77);
    l; /**bp:evaluate('l', 1);**/

    var m = SIMD.Uint8x16.replaceLane(c, 7, 88);
    m; /**bp:evaluate('m', 1);**/

    var n = SIMD.Uint8x16.replaceLane(c, 8, 99);
    n; /**bp:evaluate('n', 1);**/

    var o = SIMD.Uint8x16.replaceLane(c, 9, 100);
    o; /**bp:evaluate('o', 1);**/

    var p = SIMD.Uint8x16.replaceLane(c, 10, 110);
    p; /**bp:evaluate('p', 1);**/

    var q = SIMD.Uint8x16.replaceLane(c, 11, 111);
    q; /**bp:evaluate('q', 1);**/
}

testSIMDUint8x16_unaryop();
testSIMDUint8x16_binaryop();
testSIMDUint8x16_minmaxop();
testSIMDUint8x16_compareop();
testSIMDUint8x16_conversion();
testSIMDUint8x16_replaceLane();

WScript.Echo("PASS");
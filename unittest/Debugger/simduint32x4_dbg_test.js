//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

function testSIMDUint32x4_unaryop() {
    // test SIMDUint32x4 UnaryOp
    var m = SIMD.Uint32x4(16, 32, 64, 128);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE);locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Uint32x4(1, 2, 3, 4);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n', 2);**/

    var a = SIMD.Uint32x4(4, 3, 2, 1);
    var c = SIMD.Uint32x4.not(a);    // not
    c; /**bp:evaluate('c', 0, LOCALS_TYPE), evaluate('c', 2);**/

    var d = SIMD.Uint32x4.not(SIMD.Uint32x4(4, 3, 2, 1));
    d; /**bp:evaluate('d', 0, LOCALS_TYPE), evaluate('d', 2);**/
}

function testSIMDUint32x4_binaryop() {
    // test SIMDUint32x4 BinaryOp
    var m = SIMD.Uint32x4(0xAAAAAAAA, 0xAAAAAAAA, -1431655766, 0xAAAAAAAA);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE), locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Uint32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n', 2);**/

    var o = SIMD.Uint32x4.and(m, n);  // and
    o; /**bp:evaluate('o', 0, LOCALS_TYPE), evaluate('o', 2);**/

    var m1 = SIMD.Uint32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    var n1 = SIMD.Uint32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    var o1 = SIMD.Uint32x4.or(m1, n1);  // or
    o1; /**bp:evaluate('o1', 2);stack()**/

    var n2 = SIMD.Uint32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    n2; /**bp:evaluate('n2', 2);**/

    var o2 = SIMD.Uint32x4.xor(n2, o1);  // xor
    o2; /**bp:evaluate('o2', 2);**/

    var a = SIMD.Uint32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x7fffffff, 0x0);
    var b = SIMD.Uint32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c = SIMD.Uint32x4.add(a, b);
    c; /**bp:evaluate('c', 2);**/

    var a1 = SIMD.Uint32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b1 = SIMD.Uint32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c1 = SIMD.Uint32x4.sub(a1, b1);
    c1; /**bp:evaluate('c1', 2);**/

    var a2 = SIMD.Uint32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b2 = SIMD.Uint32x4(0x1, 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF);
    var c2 = SIMD.Uint32x4.mul(a2, b2);
    c2; /**bp:evaluate('c2', 2);**/
}

function testSIMDUint32x4_compareop() {
    // test SIMDUint32x4 compareOp
    var m = SIMD.Uint32x4(1000, 2000, 100, 1);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE), locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Uint32x4(2000, 2000, 1, 100);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n', 2);**/
    var cmp = SIMD.Uint32x4.lessThan(m, n);  // lessThan
    cmp; /**bp:evaluate('cmp', 0, LOCALS_TYPE), evaluate('cmp', 2);**/

    cmp = SIMD.Uint32x4.equal(m, n);         // equal
    cmp; /**bp:evaluate('cmp', 2);**/

    cmp = SIMD.Uint32x4.greaterThan(m, n);   // greaterThan
    cmp; /**bp:evaluate('cmp', 2);**/
}

function testSIMDUint32x4_minmaxop() {
    // test SIMDUint32x4 minmaxOp
    var m = SIMD.Uint32x4(1000, 2000, 100, 1);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE), locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Uint32x4(2000, 3000, 1, 200);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n', 2);**/
    var cmp = SIMD.Uint32x4.min(m, n);  // min
    cmp; /**bp:evaluate('cmp', 0, LOCALS_TYPE), evaluate('cmp', 2);**/

    cmp = SIMD.Uint32x4.max(m, n);      // max
    cmp; /**bp:evaluate('cmp', 2);**/
}

function testSIMDUint32x4_conversion() {
    // test SIMDUint32x4 Conversion
    var m = SIMD.Float32x4(1.0, 2.2, 3.6, 4.8);
    m; /**bp:locals();evaluate('m', 0, LOCALS_TYPE)**/
    var n = SIMD.Uint32x4.fromFloat32x4(m);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), locals(1);bp:evaluate('n', 2);**/

    var c = SIMD.Float32x4(1.0, 2.0, 3.0, 4.0);
    c; /**bp:evaluate('c', 0, LOCALS_TYPE), evaluate('c', 1);**/
    var d = SIMD.Uint32x4.fromFloat32x4Bits(c);
    d; /**bp:evaluate('d', 0, LOCALS_TYPE), evaluate('d', 2);**/
}

function testSIMDUint32x4_replaceLane() {
    // test SIMDUint32x4
    var c = SIMD.Uint32x4(1, 2, 3, 4);
    c; /**bp:locals();evaluate('c', 0, LOCALS_TYPE)**/

    // test SIMDUint32x4 replaceLane
    var f = SIMD.Uint32x4.replaceLane(c, 0, 20);
    f; /**bp:evaluate('f', 0, LOCALS_TYPE), evaluate('f', 1);**/

    var g = SIMD.Uint32x4.replaceLane(c, 1, 22);
    g; /**bp:evaluate('g', 2);**/

    var h = SIMD.Uint32x4.replaceLane(c, 2, 44);
    h; /**bp:evaluate('h', 2);**/

    var i = SIMD.Uint32x4.replaceLane(c, 3, 55);
    i; /**bp:locals();bp:evaluate('i', 1);**/
}

testSIMDUint32x4_unaryop();
testSIMDUint32x4_binaryop();
testSIMDUint32x4_compareop();
testSIMDUint32x4_minmaxop();
testSIMDUint32x4_conversion();
testSIMDUint32x4_replaceLane();

WScript.Echo("PASS");
//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

function testSIMDInt32x4_unaryop() {
    // test SIMDInt32x4 UnaryOp
    var m = SIMD.Int32x4(16, 32, 64, 128);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE), evaluate('m', 1);**/
    var n = SIMD.Int32x4(-1, -2, -3, -4);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n', 2);**/

    var a1 = SIMD.Int32x4.neg(m);    // neg
    a1; /**bp:evaluate('a1', 2);**/
    var a2 = SIMD.Int32x4.neg(n);
    a2; /**bp:evaluate('a2', 2);**/

    var a = SIMD.Int32x4(-4, -3, -2, -1);
    var c = SIMD.Int32x4.not(a);    // not
    c; /**bp:evaluate('c', 2);**/

    c = SIMD.Int32x4.not(SIMD.Int32x4(4, 3, 2, 1));
    c; /**bp:evaluate('c', 2);**/
}

function testSIMDInt32x4_binaryop() {
    // test SIMDInt32x4 BinaryOp
    var m = SIMD.Int32x4(0xAAAAAAAA, 0xAAAAAAAA, -1431655766, 0xAAAAAAAA);
    m; /**bp:locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    n; /**bp:evaluate('m', 0, LOCALS_TYPE), evaluate('n', 2);**/

    var o = SIMD.Int32x4.and(m, n);  // and
    o; /**bp:evaluate('m', 0, LOCALS_TYPE), evaluate('o', 2);**/

    var m1 = SIMD.Int32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    var n1 = SIMD.Int32x4(0x55555555, 0x55555555, 0x55555555, 0x55555555);
    var o1 = SIMD.Int32x4.or(m1, n1);  // or
    o1; /**bp:evaluate('o1', 2), evaluate('o1', 0, LOCALS_TYPE)**/

    var n2 = SIMD.Int32x4(0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA, 0xAAAAAAAA);
    n2; /**bp:evaluate('n2', 2);**/

    var o2 = SIMD.Int32x4.xor(n2, o1);  // xor
    o2; /**bp:evaluate('o2', 2);**/

    var a = SIMD.Int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x7fffffff, 0x0);
    var b = SIMD.Int32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c = SIMD.Int32x4.add(a, b);
    c; /**bp:evaluate('c', 2);**/

    var a1 = SIMD.Int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b1 = SIMD.Int32x4(0x1, 0xFFFFFFFF, 0x1, 0xFFFFFFFF);
    var c1 = SIMD.Int32x4.sub(a1, b1);
    c1; /**bp:evaluate('c1', 2);**/

    var a2 = SIMD.Int32x4(0xFFFFFFFF, 0xFFFFFFFF, 0x80000000, 0x0);
    var b2 = SIMD.Int32x4(0x1, 0xFFFFFFFF, 0x80000000, 0xFFFFFFFF);
    var c2 = SIMD.Int32x4.mul(a2, b2);
    c2; /**bp:evaluate('c2', 2);**/
}

function testSIMDInt32x4_compareop() {
    // test SIMDInt32x4 compareOp
    var m = SIMD.Int32x4(1000, 2000, 100, 1);
    m; /**bp:locals(1);bp:evaluate('m', 1);**/
    var n = SIMD.Int32x4(2000, 2000, 1, 100);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n', 2);**/
    var cmp = SIMD.Int32x4.lessThan(m, n);  // lessThan
    cmp; /**bp:evaluate('cmp', 2);**/

    cmp = SIMD.Int32x4.equal(m, n);         // equal
    cmp; /**bp:evaluate('cmp', 2);**/

    cmp = SIMD.Int32x4.greaterThan(m, n);   // greaterThan
    cmp; /**bp:evaluate('cmp', 2);**/
}

function testSIMDInt32x4_conversion() {
    // test SIMDInt32x4 Conversion
    var m = SIMD.Float32x4(1.0, 2.2, 3.6, 4.8);
    m; /**bp:locals();evaluate('m', 0, LOCALS_TYPE)**/
    var n = SIMD.Int32x4.fromFloat32x4(m);
    n; /**bp:locals(1);bp:evaluate('n', 2);**/

    var c = SIMD.Float32x4(1.0, 2.0, 3.0, 4.0);
    c; /**bp:evaluate('c', 1);**/
    var d = SIMD.Int32x4.fromFloat32x4Bits(c);
    d; /**bp:evaluate('d', 2);**/
}

function testSIMDInt32x4_replaceLane() {
    // test SIMDInt32x4
    var c = SIMD.Int32x4(1, 2, 3, 4);
    c; /**bp:locals();evaluate('c', 0, LOCALS_TYPE)**/

    // test SIMDInt32x4 WithLane......
    var f = SIMD.Int32x4.replaceLane(c, 0, 20);
    f; /**bp:evaluate('f', 1);**/

    var g = SIMD.Int32x4.replaceLane(c, 1, 22);
    g; /**bp:evaluate('g', 2);**/

    var h = SIMD.Int32x4.replaceLane(c, 2, 44.0);
    h; /**bp:evaluate('h', 2);**/

    var i = SIMD.Int32x4.replaceLane(c, 3, 55.5);
    i; /**bp:locals();bp:evaluate('i', 1);**/
}

testSIMDInt32x4_unaryop();
testSIMDInt32x4_binaryop();
testSIMDInt32x4_compareop();
testSIMDInt32x4_conversion();
testSIMDInt32x4_replaceLane();

WScript.Echo("PASS");
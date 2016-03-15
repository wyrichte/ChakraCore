//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

// SIMDFloat32x4
function testSIMDFloat32x4_unaryop() {
    // test SIMDFloat32x4 unary ops
    var a = SIMD.Float32x4(-4.0, -3.0, -2.0, -1.0);
    a; /**bp:locals();evaluate('a', 0, LOCALS_TYPE)**/
    var c = SIMD.Float32x4.abs(a);                      // abs
    c; /**bp:evaluate('c', 0, LOCALS_TYPE); evaluate('c', 2);**/

    var a1 = SIMD.Float32x4(-4.0, -3.0, -2.0, -1.0);
    a1; /**bp:evaluate('a1', 0, LOCALS_TYPE)**/
    var c1 = SIMD.Float32x4.neg(a1);                    // neg
    c1; /**bp:evaluate('c1', 0, LOCALS_TYPE); evaluate('c1', 2);**/
}

function testSIMDFloat32x4_binaryop() {
    // test SIMDFloat32x4 binary ops
    var a = SIMD.Float32x4(4.0, 3.0, 2.0, 1.0);
    a; /**bp:locals();evaluate('a', 0, LOCALS_TYPE)**/
    var b = SIMD.Float32x4(10.0, 20.0, 30.0, 40.0);
    b; /**bp:evaluate('b', 2);**/
    var c = SIMD.Float32x4.add(a, b);                   // add
    c; /**bp:evaluate('c', 2);**/

    var a1 = SIMD.Float32x4(4.0, 3.0, 2.0, 1.0);
    var b1 = SIMD.Float32x4(10.0, 20.0, 30.0, 40.0);
    var c1 = SIMD.Float32x4.sub(a1, b1);                // sub
    c1; /**bp:evaluate('c1', 2);**/

    var a2 = SIMD.Float32x4(4.0, 3.0, 2.0, 1.0);
    var b2 = SIMD.Float32x4(10.0, 20.0, 30.0, 40.0);
    var c2 = SIMD.Float32x4.mul(a2, b2);                // mul
    c2; /**bp:evaluate('c2', 2);**/

    var a3 = SIMD.Float32x4(4.0, 9.0, 8.0, 1.0);
    var b3 = SIMD.Float32x4(2.0, 3.0, 1.0, 0.5);
    var c3 = SIMD.Float32x4.div(a3, b3);                // div
    c3; /**bp:evaluate('c3', 2);**/
}

function testSIMDFloat32x4_clamp() {
    // test SIMDFloat32x4 clamp
    var a = SIMD.Float32x4(-20.0, 10.0, 30.0, 0.5);
    a; /**bp:locals();evaluate('a', 0, LOCALS_TYPE)**/

    var lower = SIMD.Float32x4(2.0, 1.0, 50.0, 0.0);
    lower; /**bp:evaluate('lower', 0, LOCALS_TYPE); evaluate('lower', 2);**/

    var upper = SIMD.Float32x4(2.5, 5.0, 55.0, 1.0);
    upper; /**bp:evaluate('upper', 0, LOCALS_TYPE); evaluate('upper', 2);**/

    var c = SIMD.Float32x4.clamp(a, lower, upper);
    c; /**bp:evaluate('c', 0, LOCALS_TYPE); evaluate('c', 2);**/
}

function testSIMDFloat32x4_compareop() {
    // test SIMDFloat32x4 compare ops
    var m = SIMD.Float32x4(1.0, 2.0, 0.1, 0.001);
    var n = SIMD.Float32x4(2.0, 2.0, 0.001, 0.1);
    n; /**bp:locals();evaluate('n', 0, LOCALS_TYPE)**/
    var cmp = SIMD.Float32x4.lessThan(m, n);            // lessThan
    cmp; /**evaluate('cmpn', 0, LOCALS_TYPE); bp:evaluate('cmp', 2);**/

    var cmp2 = SIMD.Float32x4.lessThanOrEqual(m, n);     // lessThanOrEqual
    cmp2; /**bp:evaluate('cmp2', 2);**/

    var cmp3 = SIMD.Float32x4.equal(m, n);               // equal
    cmp3; /**bp:evaluate('cmp3', 2);**/

    var cmp4 = SIMD.Float32x4.notEqual(m, n);            // notEqual
    cmp4; /**bp:evaluate('cmp4', 2);**/

    var cmp5 = SIMD.Float32x4.greaterThanOrEqual(m, n);  // greaterThanOrEqual
    cmp5; /**bp:evaluate('cmp5', 2);**/

    var cmp6 = SIMD.Float32x4.greaterThan(m, n);         // greaterThan
    cmp6; /**bp:evaluate('cmp6', 2);**/
}

function testSIMDFloat32x4_conversion() {
    // test SIMDFloat32x4 conversion
    var o = SIMD.Int32x4(1, 2, 3, 4);
    o; /**bp:evaluate('o', 0, LOCALS_TYPE); evaluate('o', 2);**/
    var p = SIMD.Float32x4.fromInt32x4(o);
    p; /**bp:evaluate('p', 0, LOCALS_TYPE); evaluate('p', 2);**/

    var r = SIMD.Int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000);
    r; /**bp:evaluate('r', 0, LOCALS_TYPE);**/
    var s = SIMD.Float32x4.fromInt32x4Bits(r);
    s; /**bp:evaluate('s', 0, LOCALS_TYPE); evaluate('s', 2);**/
}

function testSIMDFloat32x4_replaceLane() {
    // test SIMDFloat32x4
    var c = SIMD.Float32x4(1.1, 2.2, 3.3, 4.4);
    /**bp:evaluate('c', 0, LOCALS_TYPE); locals();**/

    var e = SIMD.Float32x4.splat(4.5);  // splat
    /**bp:evaluate('e', 0, LOCALS_TYPE); evaluate('e', 2);**/

    // test SIMDFloat32x4 replaceLane
    var f = SIMD.Float32x4.replaceLane(e, 0, 20.5);  
    f; /**bp:evaluate('f', 1);**/

    var g = SIMD.Float32x4.replaceLane(e, 1, 20.0);
    g; /**bp:evaluate('g', 2);**/

    var h = SIMD.Float32x4.replaceLane(c, 2, 44.0);
    h; /**bp:evaluate('h', 2);**/

    var i = SIMD.Float32x4.replaceLane(e, 3, 55.5);
    i; /**bp:evaluate('i', 2);**/
}

testSIMDFloat32x4_unaryop();
testSIMDFloat32x4_binaryop();

testSIMDFloat32x4_compareop();
testSIMDFloat32x4_conversion();
testSIMDFloat32x4_replaceLane();

WScript.Echo("PASS");


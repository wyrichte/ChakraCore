//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

function testSIMDBool16x8() {
    // test SIMDBool16x8
    var m = SIMD.Bool16x8(true, false, true, false, true, true, false, false);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE)**/

    var a = SIMD.Int16x8(1, 2, 3, 4, 5, 6, 7, 8);
    a; /**bp:evaluate('a', 0, LOCALS_TYPE), evaluate('a',1)**/

    var b = SIMD.Int16x8(-1, -2, -3, -4, -5, -6, -7, -8);
    b; /**bp:evaluate('b', 0, LOCALS_TYPE), evaluate('b',1)**/
    var s = SIMD.Int16x8.select(m, a, b);
    s; /**bp:evaluate('b', 0, LOCALS_TYPE), locals(1)**/

    var t0 = SIMD.Int16x8.extractLane(s, 0); // 1
    t0; /**bp:evaluate('t0', 0, LOCALS_TYPE)**/
    var t1 = SIMD.Int16x8.extractLane(s, 1); // -2 
    t1; /**bp:evaluate('t1', 0, LOCALS_TYPE)**/
    var t2 = SIMD.Int16x8.extractLane(s, 2); // 3
    t2; /**bp:evaluate('t2', 0, LOCALS_TYPE)**/
    var t3 = SIMD.Int16x8.extractLane(s, 3); // -4
    t3; /**bp:evaluate('t3', 0, LOCALS_TYPE)**/

    var t4 = SIMD.Int16x8.extractLane(s, 4); // 5
    t4; /**bp:evaluate('t4', 0, LOCALS_TYPE)**/
    var t5 = SIMD.Int16x8.extractLane(s, 5); // 6 
    t5; /**bp:evaluate('t5', 0, LOCALS_TYPE)**/
    var t6 = SIMD.Int16x8.extractLane(s, 6); // -7
    t6; /**bp:evaluate('t6', 0, LOCALS_TYPE)**/
    var t7 = SIMD.Int16x8.extractLane(s, 7); // -8
    t7; /**bp:evaluate('t7', 0, LOCALS_TYPE)**/
}

function testSIMDBool16x8_splat() {
    // test SIMDBool16x8 splat
    var a = SIMD.Bool16x8.splat(true);
    a; /**bp:evaluate('a', 0, LOCALS_TYPE), locals(1)**/

    var b = SIMD.Bool16x8.splat(false);
    b; /**bp:evaluate('b', 0, LOCALS_TYPE), evaluate('b',1)**/

    var c = SIMD.Bool16x8.splat(a);
    c; /**bp:evaluate('c', 0, LOCALS_TYPE), evaluate('c',1)**/
}

function testSIMDBool16x8_not() {
    // test SIMDBool16x8 not
    var m = SIMD.Bool16x8.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE), evaluate('m',1)**/

    var n = SIMD.Bool16x8.not(m);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n',1)**/

    var u = SIMD.Bool16x8(true, false, true, false, true, true, false, false);
    u; /**bp:evaluate('u', 0, LOCALS_TYPE), evaluate('u',1)**/

    var w = SIMD.Bool16x8.not(u);
    w; /**bp:evaluate('w', 0, LOCALS_TYPE), evaluate('w',1)**/
}

function testSIMDBool16x8_allTrue() {
    // test SIMDBool16x8 allTrue
    var m = SIMD.Bool16x8.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE), locals(1)**/

    var n = SIMD.Bool16x8.allTrue(m);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n',1)**/

    var f = SIMD.Bool16x8.not(m);
    f; /**bp:evaluate('f',1)**/

    var g = SIMD.Bool16x8.allTrue(f);
    g; /**bp:evaluate('g',1)**/

    var u = SIMD.Bool16x8(true, false, true, false, true, true, false, false);
    u; /**bp:evaluate('u',1)**/

    var w = SIMD.Bool16x8.allTrue(u);
    w; /**bp:evaluate('w',1)**/
}

function testSIMDBool16x8_anyTrue() {
    // test SIMDBool16x8 anyTrue
    var m = SIMD.Bool16x8.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE)**/

    var n = SIMD.Bool16x8.anyTrue(m);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n',1)**/

    var f = SIMD.Bool16x8.not(m);
    f; /**bp:evaluate('f',1)**/

    var g = SIMD.Bool16x8.anyTrue(f);
    g; /**bp:evaluate('g', 0, LOCALS_TYPE), evaluate('g',1)**/

    var u = SIMD.Bool16x8(true, false, true, false, true, true, false, false);
    u; /**bp:evaluate('u', 0, LOCALS_TYPE), evaluate('u',1)**/

    var w = SIMD.Bool16x8.anyTrue(u);
    w; /**bp:evaluate('m', 0, LOCALS_TYPE), evaluate('w',1)**/
}


testSIMDBool16x8();
testSIMDBool16x8_splat();
testSIMDBool16x8_not();
testSIMDBool16x8_allTrue();
testSIMDBool16x8_anyTrue();

WScript.Echo("PASS");
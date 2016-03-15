//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

function testSIMDBool32x4() {
    // test SIMDBool32x4
    var m = SIMD.Bool32x4(true, false, true, false);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE)**/
    
    var a = SIMD.Int32x4(1, 2, 3, 4);
    a; /**bp:evaluate('a', 0, LOCALS_TYPE), evaluate('a',1)**/
    var b = SIMD.Int32x4(-1, -2, -3, -4);
    b; /**bp:evaluate('b', 0, LOCALS_TYPE), evaluate('b',1)**/
    var s = SIMD.Int32x4.select(m, a, b);
    s; /**bp:locals(1)**/
    var t0 = SIMD.Int32x4.extractLane(s, 0); // 1
    t0; /**bp:evaluate('t0', 0, LOCALS_TYPE)**/
    var t1 = SIMD.Int32x4.extractLane(s, 1); // -2
    t1; /**bp:evaluate('t1', 0, LOCALS_TYPE)**/
    var t2 = SIMD.Int32x4.extractLane(s, 2); // 2
    t2; /**bp:evaluate('t2', 0, LOCALS_TYPE)**/
    var t3 = SIMD.Int32x4.extractLane(s, 3); // -4
    t3; /**bp:evaluate('t3', 0, LOCALS_TYPE)**/
}

function testSIMDBool32x4_splat() {
    // test SIMDBool32x4 splat
    var a = SIMD.Bool32x4.splat(true);
    a; /**bp:evaluate('a', 0, LOCALS_TYPE), locals(1)**/

    var b = SIMD.Bool32x4.splat(false);
    b; /**bp:evaluate('b', 0, LOCALS_TYPE), evaluate('b',1)**/

    var c = SIMD.Bool32x4.splat(b);
    c; /**bp:evaluate('c', 0, LOCALS_TYPE), evaluate('c',1)**/
}

function testSIMDBool32x4_not() {
    // test SIMDBool32x4 not
    var m = SIMD.Bool32x4.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE)**/

    var n = SIMD.Bool32x4.not(m);
    n; /**bp:evaluate('n',1)**/

    var u = SIMD.Bool32x4(true, false, true, false);
    u; /**bp:evaluate('u',1)**/

    var w = SIMD.Bool32x4.not(u);
    w; /**bp:evaluate('w',1)**/
}

function testSIMDBool32x4_allTrue() {
    // test SIMDBool32x4 allTrue
    var m = SIMD.Bool32x4.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE), locals(1)**/

    var n = SIMD.Bool32x4.allTrue(m);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n',1)**/

    var f = SIMD.Bool32x4.not(m);
    f; /**bp:evaluate('f',1)**/

    var g = SIMD.Bool32x4.allTrue(f);
    g; /**bp:evaluate('g',1)**/

    var u = SIMD.Bool32x4(true, false, true, false);
    u; /**bp:evaluate('u',1)**/

    var w = SIMD.Bool32x4.allTrue(u);
    w; /**bp:evaluate('w',1)**/
}

function testSIMDBool32x4_anyTrue() {
    // test SIMDBool32x4 anyTrue
    var m = SIMD.Bool32x4.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE)**/

    var n = SIMD.Bool32x4.anyTrue(m);
    n; /**bp:evaluate('n', 0, LOCALS_TYPE), evaluate('n',1)**/

    var f = SIMD.Bool32x4.not(m);
    f; /**bp:evaluate('f',1)**/

    var g = SIMD.Bool32x4.anyTrue(f);
    g; /**bp:evaluate('g',1)**/

    var u = SIMD.Bool32x4(true, false, true, false);
    u; /**bp:evaluate('u',1)**/

    var w = SIMD.Bool32x4.anyTrue(u);
    w; /**bp:evaluate('w',1)**/
}

testSIMDBool32x4();
testSIMDBool32x4_splat();
testSIMDBool32x4_not();
testSIMDBool32x4_allTrue();
testSIMDBool32x4_anyTrue();

WScript.Echo("PASS");
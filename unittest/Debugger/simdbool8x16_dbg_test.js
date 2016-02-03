//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//---------------------------------------------------------------------------

function testSIMDBool8x16() {
    // test SIMDBool8x16
    var m = SIMD.Bool8x16(true, false, true, false, true, true, true, true, false, false, false, false, true, true, true, true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE)**/
   
    var a = SIMD.Int8x16(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    a; /**bp:evaluate('a', 0, LOCALS_TYPE), evaluate('a',1)**/
   
    var b = SIMD.Int8x16(-1, -2, -3, -4, -5, -6, -7, -8, -9, -10, -11, -12, -13, -14, -15, -16);
    b; /**bp:evaluate('b', 0, LOCALS_TYPE), evaluate('b',1)**/
   
    var s = SIMD.Int8x16.select(m, a, b);
    s; /**bp:evaluate('s', 0, LOCALS_TYPE), locals(1)**/
    

    var t0 = SIMD.Int8x16.extractLane(s, 0); // 1
    t0; /**bp:evaluate('t0', 0, LOCALS_TYPE)**/
    var t1 = SIMD.Int8x16.extractLane(s, 1); // -2 
    t1; /**bp:evaluate('t1', 0, LOCALS_TYPE)**/
    var t2 = SIMD.Int8x16.extractLane(s, 2); // 3
    t2; /**bp:evaluate('t2', 0, LOCALS_TYPE)**/
    var t3 = SIMD.Int8x16.extractLane(s, 3); // -4
    t3; /**bp:evaluate('t3', 0, LOCALS_TYPE)**/
    
    var t4 = SIMD.Int8x16.extractLane(s, 4); // 5
    t4; /**bp:evaluate('t4', 0, LOCALS_TYPE)**/
    var t5 = SIMD.Int8x16.extractLane(s, 5); // 6 
    t5; /**bp:evaluate('t5', 0, LOCALS_TYPE)**/
    var t6 = SIMD.Int8x16.extractLane(s, 6); // 7
    t6; /**bp:evaluate('t6', 0, LOCALS_TYPE)**/
    var t7 = SIMD.Int8x16.extractLane(s, 7); // 8
    t7; /**bp:evaluate('t7', 0, LOCALS_TYPE)**/
    
    var t8 = SIMD.Int8x16.extractLane(s, 8); // -9
    t8; /**bp:evaluate('t8', 0, LOCALS_TYPE)**/
    var t9 = SIMD.Int8x16.extractLane(s, 9); // -10 
    t9; /**bp:evaluate('t9', 0, LOCALS_TYPE)**/
    var t10 = SIMD.Int8x16.extractLane(s, 10); // -11
    t10; /**bp:evaluate('t10', 0, LOCALS_TYPE)**/
    var t11 = SIMD.Int8x16.extractLane(s, 11); // -12
    t11; /**bp:evaluate('t11', 0, LOCALS_TYPE)**/
    
    var t12 = SIMD.Int8x16.extractLane(s, 12); // 13
    t12; /**bp:evaluate('t12', 0, LOCALS_TYPE)**/
    var t13 = SIMD.Int8x16.extractLane(s, 13); // 14 
    t13; /**bp:evaluate('t13', 0, LOCALS_TYPE)**/
    var t14 = SIMD.Int8x16.extractLane(s, 14); // 15
    t14; /**bp:evaluate('t14', 0, LOCALS_TYPE)**/
    var t15 = SIMD.Int8x16.extractLane(s, 15); // 16
    t15; /**bp:evaluate('t15', 0, LOCALS_TYPE)**/
}

function testSIMDBool8x16_splat() {
    // test SIMDBool8x16 splat
    var a = SIMD.Bool8x16.splat(true);
    a; /**bp:evaluate('a', 0, LOCALS_TYPE), locals(1)**/

    var b = SIMD.Bool8x16.splat(false);
    b; /**bp:evaluate('b', 0, LOCALS_TYPE), evaluate('b',1)**/

    var c = SIMD.Bool8x16.splat(b);
    c; /**bp:evaluate('a', 0, LOCALS_TYPE), evaluate('c',1)**/
}

function testSIMDBool8x16_not() {
    // test SIMDBool8x16 not
    var m = SIMD.Bool8x16.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE)**/

    var n = SIMD.Bool8x16.not(m);
    n; /**bp:evaluate('n',1)**/

    var u = SIMD.Bool8x16(true, false, true, false, true, true, true, true, false, false, false, false, true, false, true, false);
    u; /**bp:evaluate('u',1)**/

    var w = SIMD.Bool8x16.not(u);
    w; /**bp:evaluate('w', 0, LOCALS_TYPE), evaluate('w',1)**/
}

function testSIMDBool8x16_allTrue() {
    // test SIMDBool8x16 allTrue
    var m = SIMD.Bool8x16.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE), locals(1)**/

    var n = SIMD.Bool8x16.allTrue(m);
    n; /**bp:evaluate('n',1)**/

    var f = SIMD.Bool8x16.not(m);
    f; /**bp:evaluate('f',1)**/

    var g = SIMD.Bool8x16.allTrue(f);
    g; /**bp:evaluate('g',1)**/

    var u = SIMD.Bool8x16(true, false, true, false, true, true, true, true, false, false, false, false, true, false, true, false);
    u; /**bp:evaluate('u',1)**/

    var w = SIMD.Bool8x16.allTrue(u);
    w; /**bp:evaluate('w',1)**/
}

function testSIMDBool8x16_anyTrue() {
    // test SIMDBool8x16 anyTrue
    var m = SIMD.Bool8x16.splat(true);
    m; /**bp:evaluate('m', 0, LOCALS_TYPE)**/

    var n = SIMD.Bool8x16.anyTrue(m);
    n; /**bp:evaluate('n',1)**/

    var f = SIMD.Bool8x16.not(m);
    f; /**bp:evaluate('f',1)**/

    var g = SIMD.Bool8x16.anyTrue(f);
    g; /**bp:evaluate('g',1)**/

    var u = SIMD.Bool8x16(true, false, true, false, true, true, true, true, false, false, false, false, true, false, true, false);
    u; /**bp:evaluate('u',1)**/

    var w = SIMD.Bool8x16.anyTrue(u);
    w; /**bp:evaluate('w',1)**/
}


testSIMDBool8x16();
testSIMDBool8x16_splat();
testSIMDBool8x16_not();
testSIMDBool8x16_allTrue();
testSIMDBool8x16_anyTrue();

WScript.Echo("PASS");
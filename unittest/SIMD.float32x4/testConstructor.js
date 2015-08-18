function equal(a, b) {
    if (a == b)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testConstructor() {
    WScript.Echo("Constructor");
    WScript.Echo(SIMD.Float32x4 !== undefined);
    equal('function', typeof SIMD.Float32x4);

    WScript.Echo(SIMD.Float32x4(1.0, 2.0, 3.0, 4.0) !== undefined);

    var a = SIMD.Float32x4(1.0, 2.0, 3.0, 4.0);
    var b = SIMD.Float32x4.check(a);
    equal(a, b);
    try {
        var a = SIMD.Float32x4.check(1)
    }
    catch (e) {
        WScript.Echo("Type Error");
    }
}


function testFromInt32x4() {
    var m = SIMD.Int32x4(1, 2, 3, 4);
    var n = SIMD.Float32x4.fromInt32x4(m);
    WScript.Echo("FromInt32x4");
    equal(1.0, SIMD.Float32x4.extractLane(n, 0));
    equal(2.0, SIMD.Float32x4.extractLane(n, 1));
    equal(3.0, SIMD.Float32x4.extractLane(n, 2));
    equal(4.0, SIMD.Float32x4.extractLane(n, 3));
}


function testFromInt32x4Bits() {
    var m = SIMD.Int32x4(0x3F800000, 0x40000000, 0x40400000, 0x40800000);
    var n = SIMD.Float32x4.fromInt32x4Bits(m);
    WScript.Echo("FromInt32x4Bits");
    equal(1.0, SIMD.Float32x4.extractLane(n, 0));
    equal(2.0, SIMD.Float32x4.extractLane(n, 1));
    equal(3.0, SIMD.Float32x4.extractLane(n, 2));
    equal(4.0, SIMD.Float32x4.extractLane(n, 3));
}

testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();
testConstructor();

testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();
testFromInt32x4();

testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();
testFromInt32x4Bits();

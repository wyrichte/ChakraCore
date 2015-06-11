function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function equalNaN(a)
{
    if (isNaN(a))
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}
function testOr()
{
    WScript.Echo("float32x4 Or");
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.float32x4.or(a, b);
    equal(20, c.x);
    equal(28, c.y);
    equal(30, c.z);
    equalNaN(c.w);
}

function testNot()
{
    WScript.Echo("float32x4 Not");
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.float32x4.not(a, b);
    equal(-0.9999999403953552, c.x);
    equal(-1.4999998807907104, c.y);
    equal(-1.9999998807907104, c.z);
    equal(-3.999999761581421, c.w);
}

function testAnd()
{
    WScript.Echo("float32x4 And");
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.float32x4.and(a, b);

    equal(2, c.x);
    equal(2, c.y);
    equal(2, c.z);
    equal(9.4039548065783e-38, c.w);
}

function testXor()
{
    WScript.Echo("float32x4 Xor");
    var a = SIMD.float32x4(4.0, 3.0, 2.0, 1.0);
    var b = SIMD.float32x4(10.0, 20.0, 30.0, 40.0);
    var c = SIMD.float32x4.xor(a, b);

    equal(5.877471754111438e-38, c.x);
    equal(8.228460455756013e-38, c.y);
    equal(8.816207631167156e-38, c.z);
    equal(2.6584559915698317e+37, c.w);
}

testOr();
testOr();
testOr();
testOr();
testOr();
testOr();
testOr();
testOr();


testXor();
testXor();
testXor();
testXor();
testXor();
testXor();
testXor();
testXor();

testAnd();
testAnd();
testAnd();
testAnd();
testAnd();
testAnd();
testAnd();
testAnd();

testNot();
testNot();
testNot();
testNot();
testNot();
testNot();
testNot();
testNot();
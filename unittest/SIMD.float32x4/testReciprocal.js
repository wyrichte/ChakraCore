function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function almostEqual (a, b) {
    if (Math.abs(a - b) < 0.00001)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testReciprocal()
{
    WScript.Echo("float32x4 Reciprocal");
    var a = SIMD.float32x4(8.0, 4.0, 2.0, -2.0);
    var c = SIMD.float32x4.reciprocal(a);
	
    equal(0.125, c.x);
    equal(0.250, c.y);
    equal(0.5, c.z);
    equal(-0.5, c.w);
}

function testReciprocalSqrt()
{
    WScript.Echo("float32x4 ReciprocalSqrt");
    var a = SIMD.float32x4(1.0, 0.25, 0.111111, 0.0625);
    var c = SIMD.float32x4.reciprocalSqrt(a);
    almostEqual(1.0, c.x);
    almostEqual(2.0, c.y);
    almostEqual(3.0, c.z);
    almostEqual(4.0, c.w);
}


testReciprocal();
testReciprocal();
testReciprocal();
testReciprocal();
testReciprocal();
testReciprocal();
testReciprocal();

testReciprocalSqrt();
testReciprocalSqrt();
testReciprocalSqrt();
testReciprocalSqrt();
testReciprocalSqrt();
testReciprocalSqrt();
testReciprocalSqrt();
testReciprocalSqrt();

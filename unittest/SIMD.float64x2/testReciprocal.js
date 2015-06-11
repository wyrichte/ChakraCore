function equal(a, b)
{
    if (a == b)
        WScript.Echo("Correct");
    else 
        WScript.Echo(">> Fail!");
}

function almostEqual (a, b) 
{
    if (Math.abs(a - b) < 0.00001)
        WScript.Echo("Correct");
    else
        WScript.Echo(">> Fail!");
}

function testReciprocal()
{
    WScript.Echo("float64x2 Reciprocal");
    var a = SIMD.float64x2(2.0, -2.0);
    var c = SIMD.float64x2.reciprocal(a);
    equal(0.5, c.x);
    equal(-0.5, c.y);
	
    a = SIMD.float64x2(8.0, -8.0);
    c = SIMD.float64x2.reciprocal(a);
    equal(0.125, c.x);
    equal(-0.125, c.y);
}

function testReciprocalSqrt()
{
    WScript.Echo("float64x2 ReciprocalSqrt");
    var a = SIMD.float64x2(1.0, 0.25);
    var c = SIMD.float64x2.reciprocalSqrt(a);
    almostEqual(1.0, c.x);
    almostEqual(2.0, c.y);
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

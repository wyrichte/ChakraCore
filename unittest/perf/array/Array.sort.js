var IM = 139968;
var IA = 3877;
var IC = 29573;
var last = 42;

function genRandom(max) 
{ 
    return(max * (last = (last * IA + IC) % IM) / IM); 
}

function sortOrder(num1, num2)
{
    return (num1 - num2);
}

var arr1 = null;

function PreTestArraySort1()
{
    var count = TestArraySort1.count;
    arr1 = new Array(count);

    for (var i=0; i<count; ++i) {
        arr1[i] = genRandom(1.0);
    }
}

function TestArraySort1()
{
   arr1.sort(sortOrder);
}

TestArraySort1.count = 10000;
TestArraySort1.testId = "Array.sort.1";
TestArraySort1.description = "array.sort(sortOrder) (" + TestArraySort1.count + ")";
TestArraySort1.iterations = 1000;
TestArraySort1.quantifier = 100;
TestArraySort1.preCondition = PreTestArraySort1;
Register(TestArraySort1);


var arr2 = null;

function PreTestArraySort2()
{
    var count = TestArraySort2.count;
    arr2 = new Array(count);

    for (var i=0; i<count; ++i) {
        arr2[i] = genRandom(1.0);
    }
}

function TestArraySort2()
{
   arr2.sort();
}

TestArraySort2.count = 10000;
TestArraySort2.testId = "Array.sort.2";
TestArraySort2.description = "array.sort() (" + TestArraySort2.count + ")";
TestArraySort2.iterations = 10;
TestArraySort2.quantifier = 10;
TestArraySort2.preCondition = PreTestArraySort2;
Register(TestArraySort2);
var totalAverage = 0;
var testIterations = 3;

var _global;

function Register(test)
{
    if (_global == undefined)
    {
        _global = [];
    }
    _global[_global.length] = test;
}

function RunTest(test)
{
    var iter = test.iterations;
    var func = test;
    var tot = 0;
    for (var i = 0; i < testIterations; i++)
    {
        if (test.preCondition)
        {
            test.preCondition();
        }
        var start = new Date();
        for (var j = 0; j < iter; j++) {
            func();
        }
        var end = new Date();
        tot += (end - start);
    }
    return tot / testIterations;
}

function Write(s)
{
    if (typeof(WScript) == "undefined") { document.write(s + "<br/>"); }
    else { WScript.Echo(s); }
}

function PadR(s, w, c)
{
    if (s.length >= w) {
        s = s.substring(0, w);
    }
    for (var i = s.length; i < w; i++) {
        s += c;
    }
    return s;
}

function PadL(s, w, c) {
    if (s.length >= w) {
        s = s.substring(0, w);
    }
    for (var i = s.length; i < w; i++) {
        s = c + s;
    }
    return s;
}

Write(PadR("Test Name", 30, " ") + " " + PadL("Iterations", 10, " ") + " " + PadR("Elapsed", 10, " "));
Write(PadR("", 30, "-") + " " + PadL("", 10, "-") + " " + PadL("", 10, "-"));

for (var i=0;i<_global.length;i++) {
    try {
        var test = _global[i];
        if (test.testId) {
            var testAverage = RunTest(test);
            totalAverage += testAverage;
            Write(PadR(test.description, 30, " ") + " " + PadL("" + (test.iterations * test.quantifier), 10, " ") + " " + PadL("" + Math.floor(testAverage), 7, " ") + " ms");
        }
    }
    catch(e) {
    }
}

Write(PadR("", 30, "-") + " " + PadL("", 10, "-") + " " + PadL("", 10, "-"));
Write(PadR("Total", 30, " ") + " " + PadL("", 10, " ") + " " + PadL("" + Math.floor(totalAverage), 7, " ") + " ms");
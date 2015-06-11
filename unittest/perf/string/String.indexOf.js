function TestStringIndexOf1()
{
    var str = "abcdefghijklmnopqrstuvwxyz";
    var res = null;
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    res = str.indexOf("xyz");
    return res;
}
TestStringIndexOf1.testId = "String.fromCharCode.1";
TestStringIndexOf1.description = "\"abc...xyz\".indexOf(\"xyz\")";
TestStringIndexOf1.iterations = 5000;
TestStringIndexOf1.quantifier = 100;
Register(TestStringIndexOf1);
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
var a = [0, 0, 0, 0];
var o = { s0: 0, s1: 0, s2: 0, s3: 0, "0": 0, "1": 0, "2": 0, "3": 0 };
var s0 = "s0", s1 = "s1", s2 = "s2", s3 = "s3", s99 = "s99";
var n0 = 0, n1 = 1, n2 = 2, n3 = 3, n99 = 99;
var ns0 = "0", ns1 = "1", ns2 = "2", ns3 = "3", ns99 = "99";
var r = 0;

var oLargeProps = {};
var slen1 = "s";
var slen2 = slen1 + slen1;
var slen4 = slen2 + slen2;
var slen8 = slen4 + slen4;
var slen16 = slen8 + slen8;
var slen32 = slen16 + slen16;
var slen64 = slen32 + slen32;
var slen128 = slen64 + slen64;
oLargeProps[slen1] = 0;
oLargeProps[slen2] = 0;
oLargeProps[slen4] = 0;
oLargeProps[slen8] = 0;
oLargeProps[slen16] = 0;
oLargeProps[slen32] = 0;
oLargeProps[slen64] = 0;
oLargeProps[slen128] = 0;

var forInStep1 = 4, forInStep2 = 16, forInStep3 = 64;
var oForInStr1 = {}, oForInStr2 = {}, oForInStr3 = {};
for (var i = 0; i < forInStep1; ++i)
    oForInStr1["s" + i] = 0;
for (var i = 0; i < forInStep2; ++i)
    oForInStr2["s" + i] = 0;
for (var i = 0; i < forInStep3; ++i)
    oForInStr3["s" + i] = 0;

var tests = [
    [function () { o.s0; }, 1, "o.s0 (exists)"],
    [function () { o.s99; }, 1, "o.s99 (does not exist)"],
    [function () { a[0]; }, 1, "a[0] (exists)"],
    [function () { a[99]; }, 1, "a[99] (does not exist)"],
    [function () { o[0]; }, 1, "o[0] (exists)"],
    [function () { o[99]; }, 1, "o[99] (does not exist)"],
    [function () { a["0"]; }, 1, "a[\"0\"] (exists)"],
    [function () { a["99"]; }, 1, "a[\"99\"] (does not exist)"],
    [function () { o["s0"]; }, 1, "o[\"s0\"] (exists)"],
    [function () { o["s99"]; }, 1, "o[\"s99\"] (does not exist)"],
    [function () { a[n0]; }, 1, "a[n0] (exists)"],
    [function () { a[n99]; }, 1, "a[n99] (does not exist)"],
    [function () { a[ns0]; }, 1, "a[ns0] (exists)"],
    [function () { a[ns99]; }, 1, "a[ns99] (does not exist)"],
    [function () { o[s0]; }, 1, "o[s0] (exists)"],
    [function () { o[s99]; }, 1, "o[s99] (does not exist)"],
    [function () { oLargeProps[slen1]; }, 1, "oLargeProps[slen1] (exists)"],
    [function () { oLargeProps[slen2]; }, 1, "oLargeProps[slen2] (exists)"],
    [function () { oLargeProps[slen4]; }, 1, "oLargeProps[slen4] (exists)"],
    [function () { oLargeProps[slen8]; }, 1, "oLargeProps[slen8] (exists)"],
    [function () { oLargeProps[slen16]; }, 1, "oLargeProps[slen16] (exists)"],
    [function () { oLargeProps[slen32]; }, 1, "oLargeProps[slen32] (exists)"],
    [function () { oLargeProps[slen64]; }, 1, "oLargeProps[slen64] (exists)"],
    [function () { oLargeProps[slen128]; }, 1, "oLargeProps[slen128] (exists)"],
    [function () { a[n0]; a[n1]; a[n2]; a[n3]; }, 4, "a[repeatingInt] (all exist)"],
    [function () { a[ns0]; a[ns1]; a[ns2]; a[ns3]; }, 4, "a[repeatingInt] (all exist)"],
    [function () { o[n0]; o[n1]; o[n2]; o[n3]; }, 4, "o[repeatingInt] (all exist)"],
    [function () { o[s0]; o[s1]; o[s2]; o[s3]; }, 4, "o[repeatingStr] (all exist)"],
    [function () { a[repeatingGeneratedInt()]; }, 1, "a[repeatingGeneratedInt()] (all exist)"],
    [function () { o[repeatingGeneratedInt()]; }, 1, "o[repeatingGeneratedInt()] (all exist)"],
    [function () { o[repeatingGeneratedStr()]; }, 1, "o[repeatingGeneratedStr()] (all exist)"],
    [function () { a[randomInt()]; }, 1, "a[randomInt()] (none exist)"],
    [function () { o[randomInt()]; }, 1, "o[randomInt()] (none exist)"],
    [function () { o[randomStr()]; }, 1, "o[randomStr()] (none exist)"],
    [function () { forIn(oForInStr1); }, forInStep1, "o[forInStr] * " + forInStep1],
    [function () { forIn(oForInStr2); }, forInStep2, "o[forInStr] * " + forInStep2],
    [function () { forIn(oForInStr3); }, forInStep3, "o[forInStr] * " + forInStep3]
];

for (var i = 0; i < tests.length; ++i) {
    tests[i][0].testId = i + 1;
    tests[i][0].description = tests[i][2];
    tests[i][0].operationsPerIteration = tests[i][1];
    tests[i][0].duration = 250;
    Register(tests[i][0]);
}

function repeatingGeneratedInt() {
    return ++r & 0x3;
}

function repeatingGeneratedStr() {
    return "s" + repeatingGeneratedInt();
}

function randomInt() {
    return Math.round(Math.random() * 1000000);
}

function randomStr() {
    return "s" + randomInt();
}

function forIn(o) {
    for (var s in o)
        o[s];
}
var totalAverage = 0;
var testIterations = 3;

var _global;

function Register(test) {
    if (_global == undefined) {
        _global = [];
    }
    _global[_global.length] = test;
}

function RunTest(test) {
    var iter = test.iterations;
    var func = test;
    var tot = 0;
    for (var i = 0; i < testIterations; i++) {
        if (test.preCondition) {
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

function RunTimedTest(test) {
    var duration = test.duration;
    var operationsPerIteration = test.operationsPerIteration;
    var func = test;
    if (test.preCondition) {
        test.preCondition();
    }
    var operations = 0;
    for (var d = Date.now(); (operations & 0xfff) !== 0 || Date.now() - d < duration; operations += operationsPerIteration) {
        func();
    }
    return Math.round(operations / (duration / 1000));
}

function Write(s) {
    if (typeof (WScript) == "undefined") { document.write(s + "<br/>"); }
    else { WScript.Echo(s); }
}

function PadR(s, w, c) {
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

function IntegerToLocaleString(n) {
    var s = n.toLocaleString();
    var dotIndex = s.indexOf(".");
    if (dotIndex != -1)
        return s.substring(0, dotIndex);
    return s;
}

Write(PadR("Test Name", 40, " ") + " " + PadL("Iterations", 13, " ") + " " + PadL("Elapsed", 10, " ") + " " + PadL("Operations/s", 13, " "));
Write(PadR("", 40, "-") + " " + PadL("", 13, "-") + " " + PadL("", 10, "-") + " " + PadL("", 13, "-"));

for (var i = 0; i < _global.length; i++) {
    try {
        var test = _global[i];
        if (test.testId) {
            if (test.iterations) {
                var testAverage = RunTest(test);
                totalAverage += testAverage;
                Write(PadR(test.description, 40, " ") + " " + PadL("" + IntegerToLocaleString(test.iterations * test.quantifier), 13, " ") + " " + PadL("" + Math.floor(testAverage), 7, " ") + " ms " + PadL("" + IntegerToLocaleString(test.iterations / (testAverage / 1000)), 13, " "));
            } else if (test.duration) {
                var operationsPerSecond = RunTimedTest(test);
                Write(PadR(test.description, 40, " ") + " " + PadL("" + IntegerToLocaleString(operationsPerSecond * (test.duration / 1000)), 13, " ") + " " + PadL("" + test.duration, 7, " ") + " ms " + PadL("" + IntegerToLocaleString(operationsPerSecond), 13, " "));
            }
        }
    }
    catch (e) {
    }
}

Write(PadR("", 40, "-") + " " + PadL("", 13, "-") + " " + PadL("", 10, "-") + " " + PadL("", 13, "-"));
Write(PadR("Total", 40, " ") + " " + PadL("", 13, " ") + " " + PadL("" + Math.floor(totalAverage), 7, " ") + " ms " + PadL("N/A", 13, " "));

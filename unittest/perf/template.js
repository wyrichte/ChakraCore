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
    if (typeof (WScript) == "undefined") 
    { 
         if(typeof(document) != "undefined")
         {
            document.write(s + "<br/>"); 
         }
         else //v8
         {
            print(s);
         }
    }
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

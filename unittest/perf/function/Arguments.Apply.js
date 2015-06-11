function TestArgumentsApply()
{
    function create(f, _this)
    {
        return function()
        {
            f.apply(_this, arguments);
        }
    }

    var obj1 = { f: function(arg) { var temp = arg; return temp; } };
    var f1 = create(obj1.f, obj1);

    f1();
    f1(1);
    f1(1, 2);
    f1(1, 2, 3);
    f1(1, 2, 3, 4);
    f1(1, 2, 3, 4, 5.5);
    f1(1, 2, 3, 4, 5.5, "a");
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });
    f1(1, 2, 3, 4, 5.5, "a", { x: "x" });

    var obj2 = { 
        f: function(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10)
        { 
            var temp = arg1; return temp; 
        } 
    };
    var f2 = create(obj2.f, obj2);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
    f2(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1);
}

TestArgumentsApply.testId = "Object.new.1";
TestArgumentsApply.description = "new Object()";
TestArgumentsApply.iterations = 25000;
TestArgumentsApply.quantifier = 100;
Register(TestArgumentsApply);

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

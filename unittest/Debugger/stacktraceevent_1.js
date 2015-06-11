// Validate the EmitStackTraceEvent functionality to emit stack trace event.

function zee() {
   WScript.EmitStackTraceEvent();
   WScript.EmitStackTraceEvent(1/*maxFrame*/);
   
    return 10;
}

function bar () {
    zee();
}
function foo() {
    bar();
}

foo();

WScript.EmitStackTraceEvent();

function runTest(test) {
    test();
}

function callBadFormat() {
    try {
        var formatter = new Intl.NumberFormat("INVALID CURRENCY CODE");
    } catch (e) {
        WScript.EmitStackTraceEvent();
    }
}

runTest(function () {
    var count = 0; // Only run once
    var array = [
        { toString: function () { if (count++ == 0) { callBadFormat(); } } },
        5
    ];
    var collator = new Intl.Collator();
    array.sort(collator.compare);
});

function evalTest() {
    var str = "function test1() {\n";
        str += "(function() {\n"
        str += "WScript.EmitStackTraceEvent();\n";
        str += "})();\n";
        str += "}\ntest1()";
        eval(str);
}
evalTest();
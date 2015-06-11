try {
    getStackTrace(1);
}
catch (e) {
    WScript.Echo(e);
}

try {
    diagnosticsScript.getStackTrace(1);
}
catch (e) {
    WScript.Echo(e);
}

var diag = WScript.LoadScriptFile("getStackTraceTest2.js", "samethread", "diagnostics");

function foo() {
    diag.ConsoleLog("for getStackTrace(0)", 0);
    diag.ConsoleLog("for getStackTrace(1)", 1);
    diag.ConsoleLog("for getStackTrace(5)", 5);
}
function bar() {
    foo();
}
bar();


function evalTest() {
    var str = "function test1() {\n";
    str += "(function() {\n"
    str += "diag.ConsoleLog('for getStackTrace(0)', 0)\n"
    str += "diag.ConsoleLog('for getStackTrace(5)', 5)\n"
    str += "diag.ConsoleLog('for getStackTrace(-1)', -1)\n"
    str += "})();\n";
    str += "}\ntest1()";
    eval(str);
}
evalTest();


function runTest(test) {
    test();
}

function callBadFormat() {
    diag.ConsoleLog("for getStackTrace(-1)", -1);
}

// Intl.Collator.prototype.compare
// Array.prototype.sort
runTest(function () {
    var count = 0; // Only run once
    var array = [
        { toString: function () { if (count++ == 0) { callBadFormat(); } } },
        5
    ];
    var collator = new Intl.Collator();
    array.sort(collator.compare);
});

var bad_locale = { toString: function () { 
    diag.ConsoleLog("for getStackTrace(-1)", -1);
} };
var bad_locales = [bad_locale];

[Intl.DateTimeFormat].forEach(function (obj) {
    runTest(function () {
        return bad_locales + "";
    });

 });

 (function () {
    var d = new Date();
    [d.toLocaleString, d.toLocaleDateString, d.toLocaleTimeString].forEach(function (f) {
        runTest(function () {
            return bad_locales + "";
        });
    });

})();

// Validate the Debug.emitStackTraceEvent functionality to emit stack trace event.

function zee() {
   Debug.emitStackTraceEvent();
   Debug.emitStackTraceEvent(121/*operationId*/, 1/*maxFrame*/);
   
    return 10;
}

function bar () {
    zee();
}
function foo() {
    bar();
}

foo();

Debug.emitStackTraceEvent();


function evalTest() {
    var str = "function test1() {\n";
        str += "(function() {\n"
        str += "Debug.emitStackTraceEvent();\n";
        str += "})();\n";
        str += "}\ntest1()";
        eval(str);
}
evalTest();
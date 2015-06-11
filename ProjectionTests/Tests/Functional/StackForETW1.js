function baz(operationName, logLevel) {
	return Debug.msTraceAsyncOperationStarting(operationName, logLevel);
}

function bar(operationName, logLevel) {
	return baz(operationName, logLevel);
}

function foo(operationName, logLevel) {
	return bar(operationName, logLevel);
}

var opId = foo("somestring", 0);
WScript.Echo(opId);

Debug.msTraceAsyncOperationStarting("atglobal", 0);

function evalTest() {
    var str = "function test1() {\n";
        str += "(function() {\n"
        str += "Debug.msTraceAsyncOperationStarting('atglobal', 0)\n";
        str += "})();\n";
        str += "}\ntest1()";
        eval(str);
}
evalTest();


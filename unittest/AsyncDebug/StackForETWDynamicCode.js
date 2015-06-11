var funcbaz = new Function("operationName", "logLevel", "return Debug.msTraceAsyncOperationStarting(operationName, logLevel);");

function foo(operationName, logLevel) {
	return bar(operationName, logLevel);
}

eval("function bar(operationName, logLevel) { return funcbaz(operationName, logLevel); }");

var opId = foo("anotherbiglongstringname", 1);

WScript.Echo(opId);

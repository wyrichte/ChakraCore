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

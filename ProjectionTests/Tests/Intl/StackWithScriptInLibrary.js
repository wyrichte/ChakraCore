// BLUE: 241876 - If the stackwalk comes upon a frame from a Chakra library function, we should 
//                not include it in the async debug stacktrace.

function baz(operationName, logLevel) {
	return Debug.msTraceAsyncOperationStarting(operationName, logLevel);
}

function bar(operationName, logLevel) {
	return baz(operationName, logLevel);
}

function foo(operationName, logLevel) {
	return bar(operationName, logLevel);
}

var coll = Intl.Collator();
var a = {
    toString: function () {
        return foo("somestring", 0);
    }
}
coll.compare(a, "b");

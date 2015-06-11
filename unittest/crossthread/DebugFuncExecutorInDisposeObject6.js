
function test(arg) {
	try {
		var x = Debug.createDebugFuncExecutorInDisposeObject(arg);
	} catch(ex) {
		WScript.Echo(ex.message);
	}
}

test(1);
test({});
test([]);
test("string");
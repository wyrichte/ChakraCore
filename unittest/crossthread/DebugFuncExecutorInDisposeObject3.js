function testcase3()
{
    WScript.Echo('inside testcase3');
    var x = Debug.createDebugFuncExecutorInDisposeObject(test, "h", {}, 1, this);
    CollectGarbage();
    CollectGarbage();

    function test() {
		WScript.Echo('inside test');
		for(var arg in arguments) {
			WScript.Echo(arguments[arg]);
		}        
    }
}

testcase3();
CollectGarbage();
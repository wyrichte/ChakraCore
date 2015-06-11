// shutdown scenario

function testcase5() {
    function test() {
        WScript.Shutdown();
        WScript.Echo('should not get executed.');
    }

    var x = Debug.createDebugFuncExecutorInDisposeObject(test);
    x = 1;
    CollectGarbage();
}
WScript.Echo('started');
testcase5();

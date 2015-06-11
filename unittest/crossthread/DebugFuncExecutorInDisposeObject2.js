//Negative test case
function testcase2() {
    try {
        var x = Debug.createDebugFuncExecutorInDisposeObject();
        x = 1;
        CollectGarbage();
    } catch (ex) {
        if (ex.message == 'Invalid function argument')
            WScript.Echo('Expected failure : ' + ex.message);
        else
            throw ex;
    }
}

testcase2();
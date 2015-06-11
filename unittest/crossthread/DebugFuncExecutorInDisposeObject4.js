// cross context case
function testcase4() {
    try {
        var scCode = "function foo() { WScript.Shutdown(this); WScript.Echo('done1'); }";
        var sc1 = WScript.LoadScriptFile('.\\DummyFileForCctx.js', 'samethread');
        sc1.id = 1;
        var x = sc1.Debug.parseFunction(scCode);
        x();
        sc1.foo();
        x = Debug.createDebugFuncExecutorInDisposeObject(sc1.foo);
        x=1;
        CollectGarbage();
    } catch (ex) {
        WScript.Echo('Expected:' + ex.message);
    }
}

testcase4();
WScript.Echo('done2');
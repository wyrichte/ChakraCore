//Switch: JsHost.exe -targeted
function test0() {
    function makeArrayLength(x) {
        return 1;
    }
    var obj0 = {};
    obj0.func1 = function (argObj2) {
        var sc1Code = "argObj2.length= makeArrayLength(1/**bp(159):resume('step_into');**/);";
        var sc1 = WScript.LoadScriptFile('dummy_cctx.js', 'samethread');
        sc1.argObj2 = argObj2;
        sc1.makeArrayLength = makeArrayLength;
        var sc1_cctx = sc1.Debug.parseFunction(sc1Code);
        sc1_cctx();
    };
    obj0.func1(1);
};
WScript.Attach(test0);
WScript.Detach(test0);
WScript.Attach(test0);

WScript.Echo("pass");
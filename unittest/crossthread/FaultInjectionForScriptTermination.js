var result = [];

var sc1Code = "WScript.SetTimeout(timeoutFunc, id * 500); \
                            function timeoutFunc() { result.push(id); }";
var totalScriptContexts = 5;
var arr = [];

// After execution of all script contexts, verify right script contexts were executed
//Only script context that shouldn't get execute is (totalScriptContexts + 1) %  FaultInjectionScriptContextToTerminateCount
WScript.SetTimeout(validator, 2 * totalScriptContexts * 500);
function validator() {
    // 0 + 1 + 3 + 4 = 8 (script context (5 + 1) % 2 = 2nd script context is not executed
    if (result.reduce(function (prev, curr) { return prev + curr }, 0) != 8) {
        WScript.Echo(result);
        WScript.Echo("FAILED");
    } else {
        WScript.Echo("PASSED");
    }
}

// Create 'totalScriptContexts' script contexts
for (var i = 0; i < totalScriptContexts; i++) {
    var sc1 = WScript.LoadScriptFile('.\\DummyFileForCctx.js', 'samethread');
    sc1.id = i;
    sc1.result = result;
    arr.push(sc1.Debug.parseFunction(sc1Code));
}

// Allocate a finalizable object and set it NULL so it will be disposed in next GC
for (var i = 0; i < 10000; i++)
{
    var a = new Int8Array(5);
    a = null;
}



CollectGarbage();

for (var i = 0; i < totalScriptContexts; i++) {
    try {
        arr[i]();
    }
    catch (ex) {
        if (ex.message !== "Can't execute code from a freed script") {
            throw ex;
        } else {
            WScript.Echo("Expected failure : " + ex.message);
        }
    }
}

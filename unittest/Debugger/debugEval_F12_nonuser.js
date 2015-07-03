//
// 752785: CAS:WebCrawler: ASSERT:  !executeFunction->IsByteCodeDebugMode()
//
//      In this bug F12 injected non-user code which we accidentally compiled into debug mode byte code,
//      breaking the assumption that library code should always be in non-debug mode.
//

// Typical F12 scenario: diag engine injects a property into main engine.
var diag = WScript.LoadScriptFile("debugEval_F12_child.js", "samethread", "diagnostics");
var glo = this;

// This simulates main engine's function called from diag engine to execute wrapped user code (user code is in eval).
function execScript() {
    // Inject _foo as ___NonUserCode___
    _debugEval("glo._foo = function() { return 1234; }", /*NonUserCode*/true);
}

WScript.Attach(function () {
    // Inject _foo in debug mode
    diag.testDebugEval(glo);
});

WScript.Detach(function () {
    // Call _foo in non-debug mode
    WScript.Echo(glo._foo() == 1234 ? "pass" : "fail");
});

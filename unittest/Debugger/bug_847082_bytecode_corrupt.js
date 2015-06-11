// TH 847082
//      Web crawler found this bug which is a result of NonUserCode debug eval (enabled
//      by latest F12) combined with a bunch other factors related to library code.
//
//      When interpreting library code (executing function), halt dispatching won't see
//      the top executing function because we skip initial library code frames. Thus the
//      top frame it sees is actually the caller frame, not the executing function.
//
//      When debugger interpreter sees bp opcode it needs original opcode to execute. Thus
//      halt dispatching retrieves original opcode from probeBackingBlock. Unfortunately
//      in this case it incorrectly reads from probeBackingBlock of "top frame" and then
//      tells interpreter to execute that opcode for "executing function". The opcode doesn't
//      exist in "executing function" at all, crashing interpreter.
//

/**exception(resume_ignore):locals();stack()**/

// Typical F12 scenario: diag engine injects a property into main engine.
var diag = WScript.LoadScriptFile("debugEval_F12_child.js", "samethread", "diagnostics");
var glo = this;

// This simulates main engine's function called from diag engine to execute wrapped user code (user code is in eval).
function execScript() {
    _debugEval("function foo(){ var x = 0; var y = 1; throw new Error('error from library'); }", /*NonUserCode*/true);
    foo();/**bp:resume('step_into');**/
    foo();/**bp:resume('step_into');**/    
}

WScript.Attach(function () {
    diag.testDebugEval(glo);
});

WScript.Echo("pass");
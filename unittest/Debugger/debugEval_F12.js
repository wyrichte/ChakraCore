// Typical F12 scenario: diag engine injects a property into main engine.

var diag = WScript.LoadScriptFile("debugEval_F12_child.js", "samethread", "diagnostics");

var x = 1;

// This simulates main engine's function called from diag engine to execute wrapped user code (user code is in eval).
function execScript()
{
  // In non-break state only global scope variables are visible.
  // In break state function scopes variables, when captured, are visible.
  // Thus, test break and non-break state separately.
  if (Debug.debuggerEnabled)
  {
    var y = 2;
    function inner()
    { 
      var z = 3;
      y; // Capture y so that it's visible for debugger and simulate user's typing "x + y" in console at break state.
      WScript.Echo("PASS"); /**bp:evaluate('_debugEval("x + y + z", false)')**/
    }
    inner();
  }
  else
  {
    var y = _debugEval("x", false);
    if (x == y) WScript.Echo("PASS");
  }
}

var glo = Function("return this")();
diag.testDebugEval(glo);

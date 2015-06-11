// jshost -DiagnosticsEngine -debuglauch -targeted

function callback(msg)
{
  return; /**bp:stack();resume('step_out');stack()**/
}

function testDebugEval()
{
  gloCallback = callback;
  var evalStr = "(function insideDebugEval(){callback()})()";
  diagnosticsScript.debugEval(evalStr, true);
}

testDebugEval();
WScript.Echo("PASS");

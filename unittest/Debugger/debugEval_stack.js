// jshost -DiagnosticsEngine

// This script is to test:
// - error.Stack with external debugEval code is on stack
// - debugger: stack on exception when external debugEval code is on stack

function callback(msg)
{
  printStackTrace(msg);
}

// This is to validate error.Stack with presense of external eval code.
function getStackTrace(msg)
{
  try {
    throw new Error(msg);
  } catch (ex) {
    return ex.stack;
  }
}

function printStackTrace(msg)
{
  var frames = getStackTrace(msg);
  frames = trimPath(frames);
  WScript.Echo(frames);
}

function trimPath(s)
{
  return s.replace(/\(.+unittest.Debugger./ig, "(");
}


function testDebugEval(isNonUserCode, tag)
{
  gloCallback = callback;
  var evalStr = "function insideDebugEval() { callback('" + tag + "') }; insideDebugEval()";
  diagnosticsScript.debugEval(evalStr, isNonUserCode);
}

function testExceptionInDebugEval(isNonUserCode)
{
/**exception(resume_ignore):stack();**/
  var evalStr = "function insideDebugEval() { throw new Error('ex inside debugEval'); }; insideDebugEval()";
  diagnosticsScript.debugEval(evalStr, isNonUserCode);
}

var evalIsExternal = "eval is external code";
testDebugEval(true, evalIsExternal);
WScript.Echo();
testDebugEval(false, "eval is user code");

testExceptionInDebugEval(true);
testExceptionInDebugEval(false);

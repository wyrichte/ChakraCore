// Used as child script by debugEval2.js.

if (diagnosticsScript.debugEval == undefined) {
  throw new Error("diagnosticsScript.debugEval must be defined. Make sure this script is run in diagnostics engine.");
}

function testDebugEval(glo)
{
  glo["_debugEval"] = diagnosticsScript.debugEval.bind(glo);
  glo.execScript();
}

var b = WScript.LoadScriptFile("1612204_1.js", "samethread", "diagnostics");
WScript.Echo(b.diagnosticsScript.debugEval.bind(this, "x=1", true)());
WScript.Echo(b.diagnosticsScript.debugEval.bind(this, "x=1", false)());
WScript.Echo(b.diagnosticsScript.debugEval.bind(this, "x=1", "abc")());
WScript.Echo(b.diagnosticsScript.debugEval.bind(this, "x=1", {})());
WScript.Echo(b.diagnosticsScript.debugEval.bind(this, "x=1")());
WScript.Echo("Pass")
var b = WScript.LoadScriptFile("1612204_1.js", "samethread");
WScript.Echo(diagnosticsScript.debugEval.bind(b, "x=1", true)());
WScript.Echo(diagnosticsScript.debugEval.bind(b, "x=1", false)());
WScript.Echo(diagnosticsScript.debugEval.bind(b, "x=1", "abc")());
WScript.Echo(diagnosticsScript.debugEval.bind(b, "x=1", {})());
WScript.Echo(diagnosticsScript.debugEval.bind(b, "x=1")());
WScript.Echo("Pass");
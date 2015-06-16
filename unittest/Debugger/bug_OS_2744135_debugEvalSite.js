var diagContext = WScript.LoadScript("var boundDebugEval;function bingUserThis(userThis){boundDebugEval=this.diagnosticsScript.debugEval.bind(userThis);};function evalUserCode(code){boundDebugEval(code,false);};function diagFunc(){throw new Error('Diag error');};",
        "samethread",
        "diagnostics",
        false);

diagContext.bingUserThis(this);

try {
    diagContext.evalUserCode("diagContext.diagFunc();", false);
} catch (e) {
    if (e.message == "Diag error") {
        WScript.Echo("pass");
    }
}

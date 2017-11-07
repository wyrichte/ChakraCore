/**ondmbp(resume_ignore):stack();evaluate("diagnosticsScript.debugEval('function f2() { return 1; }; f2()', true)")**/
function f1() {
    WScript.ChangeDOMElement();
    WScript.Echo("PASSED");
}
f1();
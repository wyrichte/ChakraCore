var inProfilerTest = true;
if (this.WScript && this.WScript.LoadScriptFile) { // Check for running in jc/jshost
    WScript.LoadScriptFile("..\\es6\\proxytest4.js");
}
else {
    WScript.Echo('failed');
}


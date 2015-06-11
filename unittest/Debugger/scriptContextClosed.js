var e = WScript.LoadScriptFile("scriptContextClosed_data.js", "samethread");

WScript.Shutdown(e);

// Run in a timeout so script engine "e" becomes actually closed
WScript.SetTimeout(function() {
    e;
    /**bp:evaluate("e", 2)**/
    WScript.Echo("pass");
}, 1000);

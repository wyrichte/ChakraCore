// Tests that internal stack frames from Intl are hidden from the debugger when an exception is thrown.
if (this.WScript) { WScript.LoadScriptFile("TrimStackTracePath.js"); }

function testFirstChanceException() {
    var formatter = new Intl.NumberFormat("INVALID CURRENCY CODE");
}

try {
    testFirstChanceException();
}
catch (ex) {
    WScript.Echo(TrimStackTracePath(ex.stack));
}
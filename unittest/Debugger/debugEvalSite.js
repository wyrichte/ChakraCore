// Load user context in different domain (domainId 1) from diagnostics script (domainId 0)
var userContext = WScript.LoadScript("", "samethread", /* Not a diagnostics engine */ "", /* Not primary */ false, /* Different domain */ 1);

// Bind diagnostics engine debugEval to user context
var debugEval = diagnosticsScript.debugEval.bind(userContext);

try {
    debugEval("y", false); // Reference Error
} catch (e) {
    WScript.Echo("Error: " + e.message);
    WScript.Echo(e.stack === undefined); // should be undefined
}

try {
    debugEval("var var = 1;", false); // Syntax Error
} catch (e) {
    WScript.Echo("Error: " + e.message);
    WScript.Echo(e.stack === undefined); // should be undefined
}

try {
    var result = debugEval("var x = 1;x;", false); // No Error
    WScript.Echo(result === 1);
} catch (e) {
    WScript.Echo("FAIL: " + e);
}

try {
    var result = debugEval("throw 1;", false); // Not a RecyclableObject
} catch (e) {
    WScript.Echo("Error: " + e);
}

try {
    var result = debugEval("throw Math;", false); // Not a RecyclableObject, throwing built-in
} catch (e) {
    WScript.Echo("Error: " + e);
    WScript.Echo(typeof e === 'string');
}

try {
    var result = debugEval("var obj = {a:1};throw obj;", false); // Not a JavaScript Error Object
} catch (e) {
    WScript.Echo("Error: " + e);
    WScript.Echo(typeof e === 'string');
}

try {
    var result = debugEval("function oos() {oos();};oos();", false); // Out of stack space
} catch (e) {
    WScript.Echo("Error: " + e.message);
    WScript.Echo(e.stack === undefined); // should be undefined
}

// Overridden Error object
try {
    debugEval("var OrigError = Error;var Error = function(msg) {this.message = msg;};Error.prototype = new OrigError();throw new Error('My Error');", false);
} catch (e) {
    WScript.Echo(e === 'Error: My Error');
    WScript.Echo(e.stack === undefined); // should be undefined
}

// Bind diagnostics engine debugEval to diagnostics context (such that execution is in same domain)
debugEval = diagnosticsScript.debugEval.bind(this); // Diagnostics context
try {
    debugEval("y", false); // same context
} catch (e) {
    WScript.Echo("Error: " + e.message);
    WScript.Echo(e.stack !== undefined); // Should have proper stack
}
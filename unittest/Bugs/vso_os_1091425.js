function assertPropertyExists(o, p, v) {
    if (!o.hasOwnProperty(p)) {
        throw new Error("Object does not have expected property '" + p + "'");
    }
    if (o[p] !== v) {
        throw new Error("Object has property '" + p + "' but its value does not match the expected value");
    }
}

function assertPropertyDoesNotExist(o, p) {
    if (o.hasOwnProperty(p)) {
        throw new Error("Object has unexpected property '" + p + "'");
    }
}

WScript.LoadScriptFile("vso_os_1091425_1.js");
WScript.LoadScriptFile("vso_os_1091425_2.js");
try {
    WScript.LoadScriptFile("vso_os_1091425_3.js");
} catch (e) {
    // unfortunatly LoadScriptFile doesn't provide us a way to get at the thrown error
    // that made it fail, instead we just get a generic Error.  Further it always prints
    // out the thrown error message and callstack.  With these two details we must use
    // a baseline to verify the expected throwing behavior of the redefinition of a
    // non-configurable global property by a global function definition.
    if (!(e instanceof Error))
        throw e;
}

WScript.Echo("Pass");

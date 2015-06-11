try {
    eval("((x = this));");
} catch(ex) {
}

try {
    // This will throw an exception.
    eval("(524288 += x);");
} catch(ex) {
}

WScript.Echo("DONE");

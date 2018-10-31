try {
    throw "expected";
}
catch {
    var foo = 12;
    /**bp:logJson('catchBlock');stack(CALLSTACK_FLAGS)**/
    WScript.Echo('PASS');
}

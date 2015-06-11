try {
    throw "expected";
}
catch (e) {
    var foo = 12;
    /**bp:logJson('catchBlock');stack(CALLSTACK_FLAGS)**/
    WScript.Echo('PASS');
}

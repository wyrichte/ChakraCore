// Tests that let/const variables appear properly as part of an
// eval() call.
function blockScopeEvalTestFunc() {
    eval("{ let b = 1; b++; /**bp:locals()**/ }");
    return 0;
}
blockScopeEvalTestFunc();
WScript.Echo("PASSED");

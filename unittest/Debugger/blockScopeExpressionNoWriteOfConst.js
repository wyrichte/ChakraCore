// Tests that attempting to modify a constant
// through expression evaluation fails.
// Bug 168330.

function blockScopeExpressionEvaluation() {
    const a = 1;
    let b = 2;
    a; /**bp:evaluate('a++');evaluate('a')**/
    b; /**bp:evaluate('b++');evaluate('b')**/
}

blockScopeExpressionEvaluation();

WScript.Echo("PASSED");
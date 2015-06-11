// validating bug 165569 (breaking on the first offset)

function foo() {
    const a = 1; /**bp:evaluate('a');**/
    
}
foo();
WScript.Echo("Pass");

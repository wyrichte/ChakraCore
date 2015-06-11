
function foo() {
    function bar() {    
    }
    bar;
    bar; /**bp:evaluate('bar', 1, LOCALS_FULLNAME)**/
    bar;
}
foo();
WScript.Echo('pass')

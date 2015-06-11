function foo(x) {
    x = 10;
    x; /**bp:evaluate('arguments[0]');locals(1);**/
}
foo([]);

WScript.Echo("Pass");
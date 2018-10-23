function foo() {
    var a = 1;
    var b = 2; /**bp:evaluate("eval('a');");**/

    // eval inside eval string. 2nd eval should not get executed
    var c = 3; /**bp:evaluate("eval('WScript.Echo(\"This line should be executed.\");eval(\"y\");')");**/

    // eval after eval. 2nd eval should not get executed
    var d = 2; /**bp:evaluate("eval('WScript.Echo(\"This should get executed.\");');eval('WScript.Echo(\"This should NOT get executed.\");');");**/
}
foo();
WScript.Echo("pass");
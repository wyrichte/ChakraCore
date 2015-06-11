eval("var x = '🇦';");

function foo() {
    var x = 1; /**bp:dumpSourceList();**/
}
WScript.Attach(foo);
WScript.Echo("Pass");

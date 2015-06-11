// Eval before attach - Eval have nested eval and new function dynamic code
eval("eval('var x1 = 1;');var f1 = new Function();f1();");

// new Function dynamic code before attach - new Function have eval inside
var f2 = new Function("eval('var x2 = 1;');");
f2();

var f3 = new Function("eval('var x3 = 1;');var f4 = new Function('var x4 = 1;'); f4();");
function foo() {
    eval("var x5 = 1;eval('var x6 = 1;');"); // Nested eval calls after attach
    f3(); // new Function dynamic code after attach
    var y; /**bp:dumpSourceList();**/
}
WScript.Attach(foo);
WScript.Detach(foo);
WScript.Attach(foo);
WScript.Echo("pass");

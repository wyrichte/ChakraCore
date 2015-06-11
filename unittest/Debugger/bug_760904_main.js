/**exception(resume_ignore):stack()**/

var diag = WScript.LoadScriptFile("bug_760904_child1.js", "samethread", "diagnostics");

function foo1() {
    diag.ConsoleLog1();
}
function foo2() {
    diag.ConsoleLog2();
}
function foo3() {
    diag.ConsoleLog3();
}
function bar() {
    WScript.SetTimeout(foo1, 10);
    WScript.SetTimeout(foo2, 20);
    WScript.SetTimeout(foo3, 30);
}

WScript.Attach(bar);


var context1 = WScript.LoadScriptFile("ConsoleScopeCC1.js", "samethread");
var context2 = WScript.LoadScriptFile("ConsoleScopeCC2.js", "samethread");

function foo() {
    context1.foo(); // Will trigger breakpoint in context1 and evaluate new variables in console scope
    context2.foo(); // Will trigger breakpoint in context2 and access variables created in previous break in context1
}

WScript.Attach(foo);

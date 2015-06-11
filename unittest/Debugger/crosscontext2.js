// Cross context validation

var x = WScript.LoadScriptFile("crosscontext2_sub.js", "samethread");
function test()
{
    var a = "in test";
    x.testFunction(foo);           /**bp:stack();locals()**/
}

function foo()
{
    var str = "in main file";
    str; /**bp:stack()**/
}

test();

WScript.Attach(test);
WScript.Echo("Pass");

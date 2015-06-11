// Cross context validation - For profiler

var x = WScript.LoadScriptFile("crosscontext1_sub.js", "samethread");
function test()
{
    var a = "in test";
    x.testFunction(foo);
}

function foo()
{
    var str = "in main file";
    str;
}

test(); 
WScript.StartProfiling(test);

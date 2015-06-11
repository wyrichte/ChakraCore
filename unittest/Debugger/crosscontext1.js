// Cross context validation - used as debuglaunch

var x = WScript.LoadScriptFile("crosscontext1_sub.js", "samethread");
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

test();           /**bp:locals(1)**/

WScript.Echo("Pass");
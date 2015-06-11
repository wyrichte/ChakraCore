// Set the debugger exception handler.
/**exception:locals();stack()**/

function f(a)
{
    JSON.parse("I AM WRONG JSON");
}

f(10)
WScript.Echo("pass");

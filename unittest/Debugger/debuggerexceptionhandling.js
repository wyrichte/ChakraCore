// Set the debugger exception handler.
/**exception:locals();stack()**/

function f(a)
{
    a += 3;  /**bp:locals();stack();setExceptionResume('ignore')**/
    a.cause.exception = 5;

    a += 4;
    a.another.exception = 5;
}

f(10)
WScript.Echo("pass");

// Set the debugger exception handler.
/**exception(resume_ignore):locals();stack()**/

function f(a)
{
    a += 3;
    a.cause.exception = 5;

    a += 4;
    a.another.exception = 5;
}

f(10)
WScript.Echo("pass");

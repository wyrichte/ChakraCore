var glo;
var doEscape = false;
function test(param)
{
    function nested()
    {
        escape();
        return param;
    }
    function escape()
    {
        if (doEscape && !glo)
            glo = arguments.callee.caller;
    }

    nested();
}


test("test3");
test("test2"); // JIT

doEscape = true;

test("test1"); // box
WScript.Echo(glo());


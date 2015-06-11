
function test()
{
    var i = 0;
    // This only create one function assignment.  The name f1 is only available inside the function
    // via LdFuncExpr
    var f = function f1() 
    {
        if (i == 0)
        {
            i++;
            return f1();
        }
        return i;
    }
    return f();
}

WScript.Echo(test());
WScript.Echo(test());


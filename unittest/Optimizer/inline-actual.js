
function f(a)
{
    return a;
}

if (f(1,2,3,4,5,6,7,8,9,10,11,12,13,14) == f(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15))
{
    WScript.Echo("PASS");
}


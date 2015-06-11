var a = new Array;
a[1] = 100;

function func2()
{
    // Injected bailout point: test constant as an array index, which doesn't get copy prop'ed
    return a[1];
}

WScript.Echo(func2());

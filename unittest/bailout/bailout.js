var a =0;
function func()
{
    // injected bailout point #1 
    return 3;
}


// injected bailout point #2
for (var i = 0; i < 10; i++)
{
    a += func();
}

WScript.Echo(a);

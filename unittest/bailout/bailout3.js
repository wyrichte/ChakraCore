
function func3()
{
    var a = 3;
    // Bailout point #1: test const prop
    return a;
}

WScript.Echo(func3());

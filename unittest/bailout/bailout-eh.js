
var g = 1;
function test()
{
    // Bailout point 
    throw g;
}

try
{
    test()
}
catch (e)
{
    WScript.Echo(e);
}
WScript.Echo(g);

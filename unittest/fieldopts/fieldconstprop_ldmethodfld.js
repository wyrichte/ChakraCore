// Testing constant prop into LdMethodFld

function Test1()
{
    var a = new Object();
    a.x = 1;
    return a.x();       // Generate LdMethodFld, and 1 is copy proped here.
}

try
{
    Test1();
}
catch (e)
{
    WScript.Echo("PASS");
}
    

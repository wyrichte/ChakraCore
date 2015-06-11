WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");

function forLoop(obj)
{
    for (var p in obj)
    {
        WScript.Echo(p);
    }
    WScript.Echo("");
}

forLoop(x.testObject);
x.Object.prototype.p0 = "foo";
forLoop(x.testObject);
delete x.Object.prototype.p0;
forLoop(x.testObject);

WScript.Echo("--------")

forLoop(x.testObject2);
x.Object.prototype.p0 = "foo";
forLoop(x.testObject2);
delete x.Object.prototype.p0;
forLoop(x.testObject2);

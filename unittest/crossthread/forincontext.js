WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "samethread");

function forLoop(obj)
{
    i = 0;
    for (var p in obj)
    {
        if (i == 0) {
            obj.p2 = 'aaa';
        }
        if (i == 2) 
        { return p; }
        i++;
        WScript.Echo(p);
    }
    WScript.Echo("");
}

forLoop(x.numObj);

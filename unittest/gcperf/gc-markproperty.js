
function InitObject()
{

    var o = new Object();
    for (var i = 0;i < 10000; i++)
    {
	    o["a" + i] = "ab" + i;
    }
    return o;
}

var a1 = InitObject();
var a2 = InitObject();
var a3 = InitObject();
var a4 = InitObject();
var a5 = InitObject();
var a6 = InitObject();

// Primt
CollectGarbage();

// Start timing
var d = new Date();
for (var i = 0; i < 25; i++)
{
	CollectGarbage();
}
WScript.Echo((new Date() - d)/25);
	

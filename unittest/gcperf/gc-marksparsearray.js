function InitArray()
{
    var o = new Array();
    for (var i = 0;i < 10000; i++)
    {
	    o[i * 100] = i;
    }
    return o;
}

var a1 = InitArray();
var a2 = InitArray();
var a3 = InitArray();
var a4 = InitArray();

// Prime
CollectGarbage();

// Start timing
var d = new Date();
for (var i = 0; i < 25; i++)
{
	CollectGarbage();
}
WScript.Echo((new Date() - d)/25);
	

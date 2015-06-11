var link = new Object();
var next = link;
for (var i = 0; i < 1000000; i++)
{
	next.blah = new Object();
	next = next.blah;
}

// Prime
CollectGarbage();

// Start timing
var d = new Date();
for (var i = 0; i < 10; i++)
{
	CollectGarbage();
}	
WScript.Echo((new Date() - d)/10);

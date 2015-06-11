
// Prime
CollectGarbage();

// Start timing
var d = new Date();
for (var i = 0; i < 10000; i++)
{
	CollectGarbage();
}
WScript.Echo((new Date() - d)/10000);

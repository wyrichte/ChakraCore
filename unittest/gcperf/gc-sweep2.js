
var diff = 0;
var obj = new Object;
for (var i = 0; i < 10; i++)
{
	for (var j = 0; j < 50000; j++)
	{
		var s = new String("blah" + j);
		s = null;
		obj[j] = new String("blahx" + j);
	}

	var d = new Date();
	CollectGarbage();
	diff += new Date() - d;
}

WScript.Echo(diff / 10);
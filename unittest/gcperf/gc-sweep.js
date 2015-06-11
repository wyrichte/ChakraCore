
var diff = 0;
for (var i = 0; i < 10; i++)
{
	var root = new Object();
	var obj = root;
	for (var j = 0; j < 100000; j++)
	{
		obj.str = new String("blah" + j);
		obj.next = new Object();
		obj = obj.next;
	}

	root = null;
	obj = null;

	var d = new Date();
	CollectGarbage();
	diff += (new Date() - d);
}

WScript.Echo(diff / 10);

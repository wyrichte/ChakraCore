
// setup
var s = "";
for (var j = 0;	j < 10000; j++)
{
	s += j;
}

var root = new Object();
var o = root;
for (var i = 0; i < 5000; i++)
{
	o.str = s.toLowerCase();
	o.next = new Object();
	o = o.next;
}

// Prime
CollectGarbage();

// start timing
var d = new Date();
for (var i = 0; i < 25; i++)
{
	CollectGarbage();
}
WScript.Echo((new Date() - d)/25);

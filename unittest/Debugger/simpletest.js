
var a = new Array(10);
var b = new Array();
WScript.Echo("The test starts here.");
debugger 

eval('var inEval = 10;\n inEval++;\n');
WScript.Echo("InEval : " + inEval);

function foo()
{
	var str = {}
	for (var k = 0; k < 3000; k++)
	{
		str[k] = k.toString();
	}
	return str;
}
foo();
a[1] = 10;
WScript.Echo(a[1]);

debugger;

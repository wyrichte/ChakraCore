function test(x)
{
	
	for(var i=0; i< 10;i++)
	{
		x[i] = 0;
	}

	for(var i=0; i< 10;i++)
	{
		x[i] = 1;
		x[i/2] = 3;
	}

	var c = 0;

	//valid memset
	for(var i=0; i<10;i++)
	{
		x[i] = 2;
		c += x[i];	
	}
	//Invalid memset
	for(var i=0; i<9;i++)
	{
		x[i] = 3;
		c += x[i/2];	
	}

}


var x = new Array();	
test(x);

var x2 = new Array();	
test(x2);

var i;
var passed = 1;
for(var i=0; i< x.length; i++) if(x[i] != x2[i]) passed = 0;

if(passed == 1)
{
	WScript.Echo("PASSED");
}
else
{
	WScript.Echo("FAILED");
}



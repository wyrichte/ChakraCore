/*
***************PERF TEST********************
* Test for cases with non-integer values.
*/
function f(x)
{
	switch(x)
	{
		case f:break;
		case f:break;
		case f:break;
		case f:break;
		case f:break;
		case f:break;
		case f:break;
		case f:break;
		case f:break;
		case f:break;
		default:break;
	}
}

var _switchStatementStartDate = new Date();

for(i=0;i<300000;i++)
{
	f(1)
}


var _switchStatementInterval = new Date() - _switchStatementStartDate;

WScript.Echo("### TIME:", _switchStatementInterval, "ms");

 
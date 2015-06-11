function test0() 
{  
	var GiantPrintArray = [];  
	var ary = new Array();  
	ary.push(GiantPrintArray.push(ary), ary);  
	WScript.Echo(GiantPrintArray[0].toString());
}
Debug.setAutoProxyName(test0);
test0();
/* simple jump before fn return */


function foo(){
	
	var a = 1;
	a++; /**bp:setnext('bp1')**/
	WScript.Echo('Skip this marker - foo');
	
}

foo();
var b = 2;
WScript.Echo(b); /**bp(bp1):locals()**/



/**exception:stack();locals()**/

function foo(){
	var a = 100;
	x++;/**bp:setExceptionResume('ignore')**/
}

function Run(){
	WScript.SetTimeout(foo,10)
	WScript.Echo('PASSED');
}

WScript.Attach(Run);
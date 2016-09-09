/*
	Exception from within Array
*/


/**exception(resume_ignore):stack()**/


var arr = [1];
var callcount = 0;
function foo() {
    callcount++;
    if(callcount==3){
		WScript.Echo('Continuing pre exception');
		arr.forEach();
		WScript.Echo('Continuing post exception');
	}
}

function Run(){
    foo();
    foo();
	foo();
}


WScript.Attach(Run);

/*
    fast debug scenario
*/


/**exception(resume_ignore):stack();locals(1)**/

var callcount = 0;
function Run() {
    bar();
	bar();
	bar();
}

function bar() { 
    baz();
}

function baz() {   
    foo1();
}

function foo1() {   
    baz1();
}

function baz1() {
    callcount++;
    WScript.Echo(callcount);   
    if (callcount == 3) {
        throw new Error();
        WScript.Echo('Excuting post exception in baz1');
    }          
}

WScript.Attach(Run);







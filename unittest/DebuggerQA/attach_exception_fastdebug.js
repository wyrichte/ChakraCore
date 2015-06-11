/*
 foo->bar->baz
 Break on exception in baz
 fast debug scenario
*/

/**exception(resume_ignore):stack();locals()**/



var callcount = 0;

function foo() {
    bar();
}

function bar() {
    var i = 0;
    callcount++;
    if (callcount == 5)
        baz();
}

function baz() {
    x++; //ReferenceError
	WScript.Echo('Continuing post exception');
}

function Run() {
    foo();
    foo();
    foo(); //foo and bar are JITted
}

foo();
foo();
WScript.Attach(Run);



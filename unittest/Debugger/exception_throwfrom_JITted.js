/*
  Explicit Throw
*/

/**exception(resume_ignore):stack()**/

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
    callcount++;
    if (callcount == 3) {
        x++;
        WScript.Echo('Continuing post exception');
    }
}



WScript.Attach(Run);
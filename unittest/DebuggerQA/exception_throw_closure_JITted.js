/*
  ReferenceError - Closure
  x is undefined
*/

/**exception(resume_ignore):stack()**/

function foo() {
    return baz;
}

function baz() {
    var callcount = 0;
    return function () {       
        callcount = x;  //inducing error
		WScript.Echo('Continuing post exception');
    }
}


var Run = foo()();
WScript.Attach(Run);

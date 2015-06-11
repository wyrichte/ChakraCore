/*
  Implicit TypeError
*/

/**exception(resume_ignore):stack();locals()**/

var callcount = 0;

function foo() {
    var a = [1];
    callcount++;
    if (callcount == 3) {
        a.forEach() /* undefined callback induces TypeError */
        WScript.Echo('Continuing post exception');
    }
}

function Run() {
    foo();
    foo();
    foo(); //foo is JITted
}

WScript.Attach(Run);

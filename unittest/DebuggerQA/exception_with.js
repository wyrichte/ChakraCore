/*
 Throw exception from within a with statement
*/

/**exception(resume_ignore):stack()**/

var callcount = 0;

function foo() {
    callcount++;
    with({x:2}){
        if (callcount == 3) {
            y++;
			WScript.Echo('Continuing post exception');
        }
    }
}

function Run(){
    foo();
    foo();
    foo();
}


WScript.Attach(Run);

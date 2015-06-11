/*
    Exception thrown in the setter
 */

/**exception(resume_ignore):stack()**/
var callcount = 0;

var x = {
    get value() {
        return 1;
    },

    set value(val) {
        
        callcount++;
        if (callcount > 2) {
            throw new Error();
			WScript.Echo('Continuing post exception');
        }
    }
}

function foo() {
    WScript.Echo(1);
    x.value = callcount;
}

function Run() {
    foo();
    foo();
    foo();
    WScript.Echo(2);
}

WScript.Attach(Run);

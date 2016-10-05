/*
    Exception thrown in the getter
 */

/**exception(resume_ignore):stack()**/
var callcount = 0;

var x = {
    get value() {       
        callcount++;
        if(callcount>2){
            m++;
			WScript.Echo('Continuing post exception');
		}
        return 1;
    },

    set value(val){
        return true;
    }
}

function foo() {
    WScript.Echo(1);
    x.value + 1;
}

function Run() {
    foo();
    foo();
    foo();    
}

WScript.Attach(Run);
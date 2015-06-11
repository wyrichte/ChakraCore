/*
    Exception from within catch
*/

/**exception(resume_ignore):stack();locals();setFrame(1);locals()**/
var callcount = 0;
function foo() {
    var y = {
        toString: function () {
            return nonexistent++;
        }
    };

    callcount++;
    try{
        throw callcount;		
    } catch (e) {
        if (e > 2) {
           "" + e + y;
        }
    }
}



function Run() {
    foo();
    foo();
    foo();
	WScript.Echo('PASSED');
}

WScript.Attach(Run);

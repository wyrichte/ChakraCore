// Validation of the error.stack and debug break due to exception.

/**exception(resume_ignore):stack()**/

function TrimStackTracePath(line) {
    return line && line.replace(/\(.+unittest.debugger./ig, "(");
}

var f1 = function () {};

f1.sub = function () {
    obj.bar();
}

var obj = { foo : function () {         
                abc.def = 10;
            },
            bar : function () {
                try {
                    abc.def = 20;
                }
                catch(e) {
                    WScript.Echo(TrimStackTracePath(e.stack));
                }
            }
    }

f1.sub();
function Run() {
    f1.sub();
    obj.foo();
}
WScript.Attach(Run);

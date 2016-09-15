/*
    Intl
    Should not step into Intl Function - compare
    intl.compare should represent native code
*/

/**exception:stack()**/

function Run() {
    var testpass = true;

    var intl = Intl.Collator();
    intl.compare('a', 'b'); /**bp:resume('step_into')**/
    
    var afun = {
        toString: function () {           
            var err = arguments.callee.caller; /**bp:setExceptionResume('ignore')**/
			WScript.Echo('Continuing post exception-1');
            return "a"; /**bp:stack();setFrame(3);locals(1)**/
        }
    };

    var bfun = {
        toString: function () {
            x++; /**bp:setExceptionResume('ignore')**/
			WScript.Echo('Continuing post exception-2');
            return "b"; /**bp:stack();setFrame(3);locals(1)**/
        }
    };


    var res = intl.compare(afun, bfun);
     
    WScript.Echo('PASSED'); /**bp:locals(1)**/
}


WScript.Attach(Run);

//bitsamd64\JsHost.exe -version:5 -targeted -debuglaunch ES6_intl_collator_exception.js
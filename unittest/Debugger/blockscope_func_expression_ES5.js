/*
    Blockscope Function Expression
    ES5 Mode
 */

/**exception(resume_ignore)**/
function Run() {
    var x = 1; /**bp:locals(1)**/
    var callfn = bar();
    {
        //function expression
        var bar = function () { return 'bar'; }
    }

    WScript.Echo('PASSED')/**bp:locals(2)**/
}

WScript.Attach(Run);

// Validating bug 247744

function Run() {
    var a = "foo"; 
    var b = new Array();
    var c = arguments;
    bar();
}

function bar() {
    var a = "bar";
    var b = [];
    var c = new Date();
    var d = new Map();
    c;  /**bp:        
        resume('step_out');        
        locals()
        **/
    WScript.Echo("Pass");
}

WScript.Attach(Run);

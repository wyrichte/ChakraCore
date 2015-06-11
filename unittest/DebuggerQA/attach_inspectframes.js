/*
    Inspecting Frames using attach
*/


function foo() {
    var x = 1;
    bar();
}

function bar() {
    var y = 1;
    baz();
}

function baz() {
    var z = 1;
    z++; /**loc(bp1):
        locals(1);
        setFrame(1);
        locals(1);
        stack();
        setFrame(2);
        locals(1);
        stack();
        evaluate('x = 2');
        locals(1);
        setFrame(1);
        evaluate('y = 2');
        locals(1);
        **/
}

function Run() {
    foo();
    foo();
    //JIT
    foo();/**bp:enableBp('bp1')**/
	foo; /**disableBp('bp1')**/
    WScript.Echo('PASSED'); 
}

WScript.Attach(Run);
WScript.Detach(Run);


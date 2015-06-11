/*
 foo->bar->baz
 Stepping out to bar from baz
 Stepping out to foo from bar
 Observe locals
*/


function foo() {
    let x = 1;
    bar();
    x;
}

function bar() {
    let x = 2;
    baz();
    x;
}

function baz() {
    let x = 3;
    x++; 
    x;/**loc(bp1):
        locals(1);
        stack();
        resume('step_out');      
        **/
    x;
}

function Run() {
    foo();
    foo();
    //Debug JIT
    foo();/**bp:enableBp('bp1')**/
    foo; /**bp:disableBp('bp1')**/
    WScript.Echo('PASSED');
}

foo();
foo();
//JIT
WScript.Attach(Run);







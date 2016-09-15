/*
    Step over JITted function
    from a JITted function
*/

function foo() {
    var _foo = "foo";
    bar();
}

function bar() {
    var _bar = [];
    baz();/**loc(bp1):
        locals(1);
        stack();
        resume('step_over')
        **/
}

function baz(){
    var x = 3;
    x++; 
}

function Run() {
    foo();
    foo();
    //debug JIT
    foo(); /**bp:enableBp('bp1')**/
    foo; /**bp:disableBp('bp1')**/
    WScript.Echo('PASSED');
}

foo();
foo();
//JIT
WScript.Attach(Run);
WScript.Detach(Run);
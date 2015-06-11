/*
	Inspecting frames in let/const
*/

function Run() {
    let a = "foo";    
    bar();
    WScript.Echo(1);
}

function bar() {
    let a = "bar";
    baz();
    WScript.Echo(2);
}

function baz() {
    let a = "baz";
    a; 
    /**loc(bp1):
        locals(1);
        setFrame(1);
        locals(1);
        setFrame(2);
        locals(1)
   **/    
    
    WScript.Echo(3);
}
Run();/**bp:enableBp('bp1')**/
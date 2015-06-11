/*
    Step into - Step Out
	Let Const scenario
*/

function foo() {
    let a = "foo";    
}

function bar() {
    let y = 10;
    const z = 10;
    {
        let  z = 10;
        foo();/**loc(bp1):
            resume('step_into');
            locals(1);
			stack();
            resume('step_out');
            locals(1)
            **/
    }
    y;
    y; /**loc(bp2):locals(1)**/
}

function Run(){
	bar(); 
    bar();
    bar(); /**bp:enableBp('bp1');enableBp('bp2')**/
	bar; /**bp:disableBp('bp1')**/
    WScript.Echo('PASSED'); 
}

WScript.Attach(Run);

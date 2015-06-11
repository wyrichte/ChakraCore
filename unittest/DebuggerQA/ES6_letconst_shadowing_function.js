/*
    Let/Const decls inside a function body shadowing outer decls
*/

function foo(){
    var a = 1;
    const b = 100;
    foo();
    function foo() {
        const a = 100;
        let b = 2;    
        b++;
        b; /**loc(bp1):locals(1);stack()**/
    }
    WScript.Echo('PASSED'); /**loc(bp2):locals(1)**/
}

function Run(){
    foo();
    foo();
    foo();/**bp:enableBp('bp1');enableBp('bp2')**/
	foo;/**bp:disableBp('bp1');disableBp('bp2')**/
}

WScript.Attach(Run);

/*
	Let Const inside a blocksope and inside fn body
*/


function Run(){
    var a = 1;
    foo();
    function foo() {
        const a = 1;
        {
            let a = 2;
            a++;/**bp:locals(1);stack()**/
        }
    }
    WScript.Echo(a); /**bp:locals(1)**/
}

WScript.Attach(Run);
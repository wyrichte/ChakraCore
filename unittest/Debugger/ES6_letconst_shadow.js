/*
    Shadowing of let-const variables
*/

function foo(){

    var a = 1;
    a;/**bp:locals(1);stack()**/
    {
        let a = 2;
        a;/**bp:locals(1);stack()**/
        {
            const a = 3;
            a;/**bp:locals(1);stack()**/
            {
                let a = 4;
                a;/**bp:locals(1);stack()**/
            }
        }
    }
}


function Run(){
    foo();
	WScript.Echo('PASSED');
}
WScript.Attach(Run);
WScript.Detach(Run);
WScript.Attach(Run);
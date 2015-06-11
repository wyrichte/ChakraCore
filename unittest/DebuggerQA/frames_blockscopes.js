/*
    Changing values across multiple blockscope
*/


function foo() {
    let a = 1;
    {
        let b = 2;
        {
            let c = 1;
            {
                switch(c){
                    case 1: c;/**loc(bp1):
                        locals();
                        setFrame(2);
                        locals();
                        setFrame(1);
                        locals();
                        evaluate('c=22/7')**/
                        break;
                    default: b = 100;
                        break;
                }
                WScript.Echo(b);
            }
            WScript.Echo(c);
        }
    }
}
function Run(){
	foo();
    foo();
    foo(); /**bp:enableBp('bp1')**/
	foo; /**bp:disableBp('bp1')**/
}

WScript.Attach(Run);
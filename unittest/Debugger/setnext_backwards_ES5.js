/*
    Backwards setNext and functional availablity outside scope
    c is declared if if true block and used in else
*/


function Run() {
    {
        var pass = true;
        var b = 1;
        var a = 2;
        a;/**bp:setnext('bp1')**/
        if (true) {
            b++;/**loc(bp1)**/
            function c() {
                a++;
            }
            b;/**bp:setnext('bp2')**/
        } else {
            a++;/**loc(bp3)**/
        }

        if (pass) {
            WScript.Echo('in pass');/**loc(bp2)**/
            pass = false;
            pass; /**bp:setnext('bp3')**/
        }
    }
    WScript.Echo(a);
    WScript.Echo(b);
    WScript.Echo(pass);
    WScript.Echo('PASSED');
}

WScript.Attach(Run);
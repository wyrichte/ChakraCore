/*
    Jumping within a switch - ES5
*/

function Run() {
    var x = 1;
    switch (x) {
        case 1: x;
            break;/**bp:setnext('bp1');locals()**/
        case 2: x++;/**loc(bp1)**/
            x;
            break       
    }
    x;/**bp:locals(1)**/
    WScript.Echo('PASSED');
}

WScript.Attach(Run);
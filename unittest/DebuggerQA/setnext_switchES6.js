/*
    Jumping within a switch - ES6
*/

function val(){
	return 1;
}

function Run() {    
    switch (val()) {
        case 1: let z = 10;
				z++;break;/**bp:setnext('bp1');locals()**/
        case 2: let y = 1;
				y++; /**loc(bp1)**/
            break       
    }
    val;/**bp:locals(1)**/
    WScript.Echo('PASSED');
	
}


WScript.Attach(Run);
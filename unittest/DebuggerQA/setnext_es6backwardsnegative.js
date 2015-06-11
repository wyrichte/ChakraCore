/*
    Backwards non lexical jumping 
*/

/**exception(resume_ignore):stack();locals()**/

function val(){
	return 2;
}

function Run() {    
    switch (val()) {
        case 1: let z = 10;
				z++;/**loc(bp1)**/
				break;
        case 2: let y = 1;
				y++; /**bp:setnext('bp1');locals()**/
            break       
    }
    val;/**bp:locals(1)**/
	
	WScript.Echo('PASSED');
}


WScript.Attach(Run);
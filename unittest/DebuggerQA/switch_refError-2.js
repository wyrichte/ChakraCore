/**exception(resume_ignore):stack();locals()**/

function Run(){
	var m = 3;
	switch(m){
		case 3: y++; 
				throw 2; 
				break; /**bp:locals()**/
	}
	WScript.Echo('PASSED');
}

WScript.Attach(Run)
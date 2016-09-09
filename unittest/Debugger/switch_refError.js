/**exception(resume_ignore):stack();locals()**/

function Run(){
	var m = 3;
	switch(m){
		case x: break; /**bp:locals()**/
	}
	WScript.Echo('PASSED');
}

WScript.Attach(Run)
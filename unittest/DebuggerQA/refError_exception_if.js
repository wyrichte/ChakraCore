/**exception(resume_ignore):stack();locals()**/

function Run(){
	if(true){
		x++;
		WScript.Echo('Continue in if'); /**bp:stack()**/
	}else{
		WScript.Echo('Why did it reach else');
	}
}

WScript.Attach(Run);

function Run(){

	var x = 1;
	x; /**bp:setnext('bp1');locals()**/
	
	function foo(){
		x++;
	}
	foo();
	
	WScript.Echo(x); /**loc(bp1)**/	

}


Run();

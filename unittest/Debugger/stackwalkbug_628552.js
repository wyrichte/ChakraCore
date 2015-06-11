function Run() 
{
	function handle() {
		var k = 10;
		k++; /**bp:stack()**/
	}
	
	WScript.CallFunction(handle);
}
Run();
WScript.Echo("Pass");
function foo()
{
	var s = "concat "+" string"+" test";
	for(var i=0;i<5;i++)
	{
		s = s+"dst same"+" as source";
	}
	s.charCodeAt(0);	
}
foo();
foo();
foo();
WScript.Echo("Passed");
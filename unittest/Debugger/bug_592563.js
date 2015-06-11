// Validate the reliability if we shudown the engine in between.

function foo()
{
	var k = 10; /**bp:locals();resume('step_over');locals();resume('step_over');evaluate('WScript.Shutdown()');locals();resume('step_over');locals()**/
	k++;
	k++;
	WScript.Echo("Pass");
}
var str = foo.toString();
eval(str + "\nfoo()");

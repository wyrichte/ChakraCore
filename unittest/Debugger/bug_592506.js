// Validation of the bug 592506, properties added during evaluation should be added correctly.

function test()
{
	this.ttt1 = 31;
}

function bar()
{
	var k = 1; /**bp:locals(1);evaluate("test();");locals(1)**/ 
}
bar();
WScript.Echo("Pass");
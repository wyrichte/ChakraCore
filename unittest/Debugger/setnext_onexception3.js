// Setnext statement on the single statement in the global code.

/**exception:setnext(0,9)**/
function foo()
{
	eval('debugger1');
}
foo();
WScript.Echo("pass");
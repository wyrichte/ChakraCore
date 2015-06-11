// Validation of the bug 543550

var m = Map.prototype;
m;	/**bp:evaluate('m',2)**/
WScript.Echo("Pass");

function test1()
{
	"use strict"
	function bar() {
	}
	bar; /**bp:evaluate('bar', 2)**/
}
test1();
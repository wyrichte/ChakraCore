// Validating the bug 210537, where it tries to get the returned value on the first frame of the cross-context

var f = WScript.LoadScriptFile('bug_210537_sub.js',"samethread");

function foo() {
	f.test1();
	f.test1;
}
foo();
WScript.Echo("Pass");

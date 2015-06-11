/**exception(resume_ignore,firstchance): stack() **/
var pass = false
function f()
{
	throw 5;
	throw 7;
	try {
		throw 22;
		a.b.c.d.e = 0;
	}
	finally {
		pass = true;
	}
}
try {
	f();
}
catch(ex) {
	pass = false;
}

WScript.Echo(pass ? "pass" : "fail");

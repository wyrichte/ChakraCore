function f() {
var x = 2;
x = 3;
x = 4;
x = 5;
x = 6;
x = 7;
for(var i = 0; i < 2; ++i)
{
	x = 8;
	x = 9;
	x = 10;	
}
function h()
{
	var a = 22;
	x = 11;
	x = 12;
	x = 13;
	x += a;
	function i()
	{
		eval("x += a;");
		x += 12;
		x += 13;
		x += 14;
		eval("x += a;");
		eval("x += a;");
		eval("x += a;");
		x += 15;
	}
	function j()
	{
		x += a;
		x += 12;
		x += 13;
		x += 14; x+= 14.5; x += 14.6; 
		x += 14; x+= 14.5; x += 14.6; 
		x += 14; x+= 14.5; x += 14.6; 
	}
	try {
		i();
		j();
		j = "asdasd";
		j += j;
		j += j;
	}
	catch(ex) {
	}
}
h("hello", "world");

}; 
function g()
{
	f();
	f();
}
g();
WScript.Echo("pass");
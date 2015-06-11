function foo()
{
	WScript.Echo("Proto test");
	var obj1 = {a:10,b:30};
	var obj2 = [2,4,6];
	debugger;
	obj1.__proto__ = [2];
	debugger;
	delete Object.prototype.__proto__;
	obj1.__proto__ = 31;
	debugger;
}
foo();

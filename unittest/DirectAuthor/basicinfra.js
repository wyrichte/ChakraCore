var x = {XXX: 3, qqq: { /**target:A**/rrr: 5 } };
x.X/**ml:XXX,qqq,!rrr**/X = 2;
x.qqq.r/**ml:rrr,!qqq**/ = 5;

var counter = 0;
function f(obj)
{
	for(var i = 0; i < 3; ++i,++counter)
	{
		obj["prop"+counter] = 1;
	}
}

f(x);
x./**ml:prop0,prop1,prop2,!prop3**/foo = 1;
z = x./**ml:prop0,prop1,prop2,!prop3,foo**/;

f(x);
x./**ml:prop0,prop1,prop2,prop3,prop4,prop5,!prop6,XXX,qqq,foo**/.foo;
x.qqq./**gd:A**/rrr = 5;

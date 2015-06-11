function test0()
{
    this.a = 1;
    this.b = 2;
    return undefined;
}


var obj = new test0();
obj = new test0();
obj = new test0();
obj = new test0();
obj = new test0();

WScript.Echo(obj.a);
WScript.Echo(obj.b);

obj.a = 10; //Fixed Field should have got invalidated at this point.
obj.b = 20; //Fixed Field should have got invalidated at this point.

//Print new values
WScript.Echo(obj.a);
WScript.Echo(obj.b);
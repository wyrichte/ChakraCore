var a = Debug.createTypedObject(2000, "mytype", 32, true); // use default operators
function test1() {
var b = new Function();
WScript.Echo(a instanceof Object);
WScript.Echo(a instanceof Function);
try {
  WScript.Echo(b instanceof a);
} catch(e) {
WScript.Echo(e);
}
}

function test2() {
a.foo = 20;
WScript.Echo(a.foo);
WScript.Echo(a['foo']);
a['bar'] = 42;
WScript.Echo(a['bar']);
WScript.Echo(a.bar);
a[42] = 24;
WScript.Echo(a[42]);
}


test1();
test2();


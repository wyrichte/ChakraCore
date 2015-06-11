var child = WScript.LoadScriptFile("marshalprotochild.ps", "samethread");
var e = child.e;
child.installPrototype();
for (var i in e) {
WScript.Echo(i);
}

WScript.Echo(e.blah);

var obj = child.obj;
child.installObjPrototype();
for (var i in obj) {
WScript.Echo(i);
}

WScript.Echo(obj.bar);

child.installTypedArrayPrototype();
WScript.Echo(e.foo);
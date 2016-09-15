var child = WScript.LoadScriptFile("marshalprotochild.ps", "samethread");
var e = child.e;
var f = child.f;
child.installPrototype();
f.__proto__ = {cctx : 5}; // set cross-context object as a prototype 
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
WScript.Echo(f.cctx);
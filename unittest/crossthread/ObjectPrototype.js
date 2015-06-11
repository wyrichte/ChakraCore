WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
WScript.LoadScriptFile("testObjects.js");
WScript.LoadScriptFile("dumpObject.js");

WScript.Echo("--------Case 01--------");
TestObject.prototype = x.testObject;
o = new TestObject("aaa", "bbb");
dumpObject(o, true);
dumpObject(Object.getPrototypeOf(o), true);
WScript.Echo(x.testObject.isPrototypeOf(o));
WScript.Echo(o instanceof TestObject);
WScript.Echo(o instanceof x.TestObject);

WScript.Echo("--------Case 02--------");
x.TestObject.prototype = testObject;
o = new x.TestObject("aaa", "bbb");
dumpObject(o, true);
dumpObject(Object.getPrototypeOf(o), true);
WScript.Echo(testObject.isPrototypeOf(o));
WScript.Echo(o instanceof TestObject);
WScript.Echo(o instanceof x.TestObject);

WScript.Echo("--------Case 03--------");
x.TestObject.prototype = x.testObject;
o = new x.TestObject("aaa", "bbb");
dumpObject(o, true);
dumpObject(Object.getPrototypeOf(o), true);
WScript.Echo(x.testObject.isPrototypeOf(o));
WScript.Echo(o instanceof TestObject);
WScript.Echo(o instanceof x.TestObject);

WScript.Echo("--------Case 04--------");
o = Object.create(x.testObject, { p2: { value: "ccc" }, p3: { value: "ddd" } });
dumpObject(o, true);
dumpObject(Object.getPrototypeOf(o), true);
WScript.Echo(x.testObject.isPrototypeOf(o));

WScript.Echo("--------Case 05--------");
o = x.Object.create(testObject, { p2: { value: "ccc" }, p3: { value: "ddd"} });
dumpObject(o, true);
dumpObject(Object.getPrototypeOf(o), true);
WScript.Echo(testObject.isPrototypeOf(o));

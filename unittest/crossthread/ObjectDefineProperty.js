WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
WScript.LoadScriptFile("testObjects.js");
WScript.LoadScriptFile("dumpObject.js");

WScript.Echo("--------Case 01--------");
Object.defineProperty(x.testObject, "newProperty1", { value: "test", writable: true, enumerable: true, configurable: true });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 02--------");
x.testObject.newProperty1 = "foo";
dumpObject(x.testObject, true);

WScript.Echo("--------Case 03--------");
Object.defineProperty(x.testObject, "newProperty1", { value: new RegExp(), writable: true, enumerable: true, configurable: true });
dumpObject(x.testObject, true)

WScript.Echo("--------Case 04--------");
Object.defineProperty(x.testObject, "newProperty1", { value: "test", writable: true, enumerable: false, configurable: true });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 05--------");
Object.defineProperty(x.testObject, "newProperty2", { value: x.testNumberObject1, writable: true, enumerable: true, configurable: true });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 06--------");
x.testNumberObject1 = Math.E;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 07--------");
x.testObject.newProperty2 = Math.E;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 08--------");
Object.defineProperty(x.testObject, "newProperty3", { value: testNumberObject1, writable: true, enumerable: true, configurable: true });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 09--------");
testNumberObject1 = Math.E;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 10--------");
x.testObject.newProperty3 = Math.E;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 11--------");
Object.defineProperty(x.testObject, "newProperty4", { get: x.getter, set: x.setter, enumerable: true, configurable: true });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 12--------");
x.testObject.newProperty4 = "bar";
dumpObject(x.testObject, true);
WScript.Echo(x.getterSetterValue);

WScript.Echo("--------Case 13--------");
Object.defineProperty(x.testObject, "newProperty5", { get: getter, set: setter, enumerable: true, configurable: true });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 14--------");
x.testObject.newProperty5 = "baz";
dumpObject(x.testObject, true);
WScript.Echo(getterSetterValue);

WScript.Echo("--------Case 15--------");
Object.defineProperty(x.testObject, "newProperty1", { value: "test", writable: true, enumerable: true, configurable: false });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 16--------");
delete x.testObject.newProperty1;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 17--------");
var pp = new Date(2011, 2, 31);
x.testObject.newProperty6 = pp;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 18--------");
pp.setHours(17, 36);
dumpObject(x.testObject, true);

WScript.Echo("--------Case 19--------");
delete x.testObject.newProperty6;
dumpObject(x.testObject, true);

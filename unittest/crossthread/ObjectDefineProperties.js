WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
WScript.LoadScriptFile("testObjects.js", "self");
WScript.LoadScriptFile("dumpObject.js");

WScript.Echo("--------Case 01--------");
Object.defineProperties(x.testObject, { newProperty1: { value: "test", writable: true, enumerable: true, configurable: true }, newProperty2: { value: x.testNumberObject1, writable: true, enumerable: true, configurable: true} });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 02--------");
x.testObject.newProperty1 = "foo";
dumpObject(x.testObject, true);

WScript.Echo("--------Case 03--------");
Object.defineProperties(x.testObject, { newProperty1: { value: new RegExp(), writable: true, enumerable: true, configurable: true} });
dumpObject(x.testObject, true)

WScript.Echo("--------Case 04--------");
Object.defineProperties(x.testObject, { newProperty1: { value: "test", writable: true, enumerable: false, configurable: true} });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 05--------");
x.testNumberObject1 = Math.E;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 06--------");
x.testObject.newProperty2 = Math.E;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 07--------");
Object.defineProperties(x.testObject, { newProperty3: { value: testNumberObject1, writable: true, enumerable: true, configurable: true} });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 08--------");
testNumberObject1 = Math.E;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 09--------");
x.testObject.newProperty3 = Math.E;
dumpObject(x.testObject, true);

WScript.Echo("--------Case 10--------");
Object.defineProperties(x.testObject, { newProperty4: { get: x.getter, set: x.setter, enumerable: true, configurable: true} });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 11--------");
x.testObject.newProperty4 = "bar";
dumpObject(x.testObject, true);
WScript.Echo(x.getterSetterValue);

WScript.Echo("--------Case 12--------");
Object.defineProperties(x.testObject, { newProperty5: { get: getter, set: setter, enumerable: true, configurable: true} });
dumpObject(x.testObject, true);

WScript.Echo("--------Case 13--------");
x.testObject.newProperty5 = "baz";
dumpObject(x.testObject, true);
WScript.Echo(getterSetterValue);

WScript.Echo("--------Case 14--------");
Object.defineProperties(x.testObject, { newProperty1: { value: "test", writable: false, enumerable: true, configurable: false } });
dumpObject(x.testObject, true);
x.testObject.newProperty1 = "qux"
dumpObject(x.testObject, true);

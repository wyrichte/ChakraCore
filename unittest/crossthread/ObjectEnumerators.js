WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
WScript.LoadScriptFile("testObjects.js");

function getPropertyNamesFromForIn(o) {
    var array = [];
    for (p in o) {
        array[array.length] = p;
    }
    return array;
}

WScript.Echo("--------Case 01--------");
o = new x.TestObject("aaa", "bbb");
WScript.Echo(Object.getOwnPropertyNames(o));
WScript.Echo(Object.keys(o));
//WScript.Echo(getPropertyNamesFromForIn(o));
WScript.Echo();

WScript.Echo("--------Case 02--------");
o.p4 = "foo";
WScript.Echo(Object.getOwnPropertyNames(o));
WScript.Echo(Object.keys(o));
WScript.Echo(getPropertyNamesFromForIn(o));
WScript.Echo();

WScript.Echo("--------Case 03--------");
x.TestObject.prototype.p5 = "bar";
WScript.Echo(Object.getOwnPropertyNames(o));
WScript.Echo(Object.keys(o));
//WScript.Echo(getPropertyNamesFromForIn(o));
WScript.Echo();

WScript.Echo("--------Case 04--------");
Object.defineProperty(o, "p6", { value: "qux", enumerable: true});
WScript.Echo(Object.getOwnPropertyNames(o));
WScript.Echo(Object.keys(o));
//WScript.Echo(getPropertyNamesFromForIn(o));
WScript.Echo();

WScript.Echo("--------Case 05--------");
Object.defineProperty(o, "p7", { value: "quux", enumerable: false });
WScript.Echo(Object.getOwnPropertyNames(o));
WScript.Echo(Object.keys(o));
//WScript.Echo(getPropertyNamesFromForIn(o));
WScript.Echo();

WScript.Echo("--------Case 06--------");
x.o = new TestObject("aaa", "bbb");
WScript.Echo(Object.getOwnPropertyNames(x.o));
WScript.Echo(Object.keys(x.o));
//WScript.Echo(getPropertyNamesFromForIn(x.o));
WScript.Echo();

WScript.Echo("--------Case 07--------");
x.o.p4 = "foo";
WScript.Echo(Object.getOwnPropertyNames(x.o));
WScript.Echo(Object.keys(x.o));
//WScript.Echo(getPropertyNamesFromForIn(x.o));
WScript.Echo();

WScript.Echo("--------Case 08--------");
TestObject.prototype.p5 = "bar";
WScript.Echo(Object.getOwnPropertyNames(x.o));
WScript.Echo(Object.keys(x.o));
//WScript.Echo(getPropertyNamesFromForIn(x.o));
WScript.Echo();

WScript.Echo("--------Case 09--------");
Object.defineProperty(x.o, "p6", { value: "qux", enumerable: true });
WScript.Echo(Object.getOwnPropertyNames(x.o));
WScript.Echo(Object.keys(x.o));
//WScript.Echo(getPropertyNamesFromForIn(x.o));
WScript.Echo();

WScript.Echo("--------Case 10--------");
Object.defineProperty(x.o, "p7", { value: "quux", enumerable: false });
WScript.Echo(Object.getOwnPropertyNames(x.o));
WScript.Echo(Object.keys(x.o));
//WScript.Echo(getPropertyNamesFromForIn(x.o));
WScript.Echo();

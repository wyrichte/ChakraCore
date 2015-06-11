function write(args) {
    WScript.Echo(args);
}

// Remove the __ptr64 part so that the unit test runs for x86 and amd64. Ian will change the getTypeHandlerName() method later to not return __ptr64
function getTypeHandlerName(testObj) {
    return Debug.getTypeHandlerName(testObj).replace(/ __ptr64/g, "");
}

write(getTypeHandlerName(null));

var obj1 = {};
write(getTypeHandlerName(obj1));

obj1.prop1 = "prop_value";
obj1.prop2 = 5;
write(getTypeHandlerName(obj1));

delete obj1.prop1;
write(getTypeHandlerName(obj1));

Object.defineProperty(obj1, "prop3", { get: function () { return "getter";} });
write(getTypeHandlerName(obj1));

write(getTypeHandlerName(5));

write(getTypeHandlerName("string"));
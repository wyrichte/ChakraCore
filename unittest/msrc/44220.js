function victim() {}
var bound = victim.bind(null, "foo");
Reflect.construct(bound, []);

WScript.Echo("Passed");
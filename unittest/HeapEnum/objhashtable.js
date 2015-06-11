// -version:5 -DeletedPropertyReuseThreshold:1
var o = { };

o.a = 1;
delete o.a;

o[(new Date(123123123)).toString()] = 123;

WScript.Echo(Debug.getTypeHandlerName(o));

Debug.dumpHeap(o, true, true);


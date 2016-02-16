// -version:5 -DeletedPropertyReuseThreshold:1
var o = { };

o.a = 1;
delete o.a;

// (new Date(123123123)).toString()
o["Fri Jan 02 1970 02:12:03 GMT-0800 (Pacific Standard Time)"] = 123;

WScript.Echo(Debug.getTypeHandlerName(o));

Debug.dumpHeap(o, true, true);


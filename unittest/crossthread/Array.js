WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
x.WScript.LoadScriptFile("ArrayCallbacks.js");
WScript.LoadScriptFile("testObjects.js");
WScript.LoadScriptFile("ArrayCallbacks.js");

WScript.Echo("--------Case 01--------");
WScript.Echo(x.testArray.toString());

WScript.Echo("--------Case 02--------");
WScript.Echo(x.testArray.every(everyCallback1, testObject));

WScript.Echo("--------Case 03--------");
WScript.Echo(x.testArray.every(everyCallback2, testObject));

WScript.Echo("--------Case 04--------");
WScript.Echo(x.testArray.some(someCallback1, testObject));

WScript.Echo("--------Case 05--------");
WScript.Echo(x.testArray.some(someCallback2, testObject));

WScript.Echo("--------Case 06--------");
WScript.Echo(x.testArray.forEach(forEachCallback, testObject));

WScript.Echo("--------Case 07--------");
WScript.Echo(x.testArray.map(mapCallback, testObject));

WScript.Echo("--------Case 08--------");
WScript.Echo(x.testArray.filter(filterCallback, testObject));

WScript.Echo("--------Case 09--------");
WScript.Echo(x.testArray.reduce(reduceCallback, 7));

WScript.Echo("--------Case 10--------");
WScript.Echo(x.testArray.reduceRight(reduceRightCallback, 7));

WScript.Echo("--------Case 11--------");
WScript.Echo(Array.prototype.toString.call(x.testArray));

WScript.Echo("--------Case 12--------");
WScript.Echo(Array.prototype.every.call(x.testArray, everyCallback1, testObject));

WScript.Echo("--------Case 13--------");
WScript.Echo(Array.prototype.every.call(x.testArray, everyCallback2, testObject));

WScript.Echo("--------Case 14--------");
WScript.Echo(Array.prototype.some.call(x.testArray, someCallback1, testObject));

WScript.Echo("--------Case 15--------");
WScript.Echo(Array.prototype.some.call(x.testArray, someCallback2, testObject));

WScript.Echo("--------Case 16--------");
WScript.Echo(Array.prototype.forEach.call(x.testArray, forEachCallback, testObject));

WScript.Echo("--------Case 17--------");
WScript.Echo(Array.prototype.map.call(x.testArray, mapCallback, testObject));

WScript.Echo("--------Case 18--------");
WScript.Echo(Array.prototype.filter.call(x.testArray, filterCallback, testObject));

WScript.Echo("--------Case 19--------");
WScript.Echo(Array.prototype.reduce.call(x.testArray, reduceCallback, 7));

WScript.Echo("--------Case 20--------");
WScript.Echo(Array.prototype.reduceRight.call(x.testArray, reduceRightCallback, 7));

WScript.Echo("--------Case 21--------");
WScript.Echo(x.Array.prototype.toString.call(testArray));

WScript.Echo("--------Case 22--------");
WScript.Echo(x.Array.prototype.every.call(testArray, x.everyCallback1, testObject));

WScript.Echo("--------Case 23--------");
WScript.Echo(x.Array.prototype.every.call(testArray, x.everyCallback2, testObject));

WScript.Echo("--------Case 24--------");
WScript.Echo(x.Array.prototype.some.call(testArray, x.someCallback1, testObject));

WScript.Echo("--------Case 25--------");
WScript.Echo(x.Array.prototype.some.call(testArray, x.someCallback2, testObject));

WScript.Echo("--------Case 26--------");
WScript.Echo(x.Array.prototype.forEach.call(testArray, x.forEachCallback, testObject));

WScript.Echo("--------Case 27--------");
WScript.Echo(x.Array.prototype.map.call(testArray, x.mapCallback, testObject));

WScript.Echo("--------Case 28--------");
WScript.Echo(x.Array.prototype.filter.call(testArray, x.filterCallback, testObject));

WScript.Echo("--------Case 29--------");
WScript.Echo(x.Array.prototype.reduce.call(testArray, x.reduceCallback, 7));

WScript.Echo("--------Case 30--------");
WScript.Echo(x.Array.prototype.reduceRight.call(testArray, x.reduceRightCallback, 7));


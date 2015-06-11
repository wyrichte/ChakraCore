this.WScript.LoadScriptFile("..\\es6\\observerProxy.js");
var csGlobal = this.WScript.LoadScriptFile("..\\es6\\observerProxy.js", "samethread");

var HETest = {};

HETest.obj = {};

HETest.obj.proxy = observerProxy;
HETest.obj.csProxy = csGlobal.observerProxy;

Debug.dumpHeap(HETest, /*dump log*/true, /*forbaselineCompare*/true, /*rootsOnly*/false, /*returnArray*/false);
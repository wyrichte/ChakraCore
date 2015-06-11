WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
WScript.LoadScriptFile("testObjects.js");

WScript.Echo(JSON.stringify(x.testBigObject));
WScript.Echo();
WScript.Echo(x.JSON.stringify(testBigObject));

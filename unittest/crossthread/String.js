WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
WScript.LoadScriptFile("testObjects.js");

WScript.Echo(x.testString.toString());
WScript.Echo(x.testString.valueOf());
WScript.Echo(x.testString.valueOf() == x.testString);

WScript.Echo(String.prototype.toString.call(x.testString));
WScript.Echo(String.prototype.valueOf.call(x.testString));
WScript.Echo(String.prototype.valueOf.call(x.testString) == x.testString);

WScript.Echo(x.String.prototype.toString.call(testString));
WScript.Echo(x.String.prototype.valueOf.call(testString));
WScript.Echo(x.String.prototype.valueOf.call(testString) == testString);

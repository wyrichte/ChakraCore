WScript.RegisterCrossThreadInterfacePS();

var x = WScript.LoadScriptFile("testObjects.js", "crossthread");
WScript.LoadScriptFile("testObjects.js");

WScript.Echo(x.testBoolean.toString());
WScript.Echo(x.testBoolean.valueOf());
WScript.Echo(x.testBoolean.valueOf() == x.testBoolean);

WScript.Echo(Boolean.prototype.toString.call(x.testBoolean));
WScript.Echo(Boolean.prototype.valueOf.call(x.testBoolean));
WScript.Echo(Boolean.prototype.valueOf.call(x.testBoolean) == x.testBoolean);

WScript.Echo(x.Boolean.prototype.toString.call(testBoolean));
WScript.Echo(x.Boolean.prototype.valueOf.call(testBoolean));
WScript.Echo(x.Boolean.prototype.valueOf.call(testBoolean) == testBoolean);

child = WScript.LoadScriptFile("objectchild.js", "crossthread");
WScript.CallFunction(child.foo);
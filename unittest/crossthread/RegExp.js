var x = WScript.LoadScriptFile("RegExp.TestObjects.js", "samethread");
x.rep.exec("this");
WScript.Echo(x.RegExp.$1);
WScript.Echo("Done!");
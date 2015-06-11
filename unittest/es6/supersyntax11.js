var baz = Function("(z => super())();");
WScript.Echo("statements before super reference executed as expected");
baz();
WScript.Echo("ERROR:statements after super reference should not be executed");
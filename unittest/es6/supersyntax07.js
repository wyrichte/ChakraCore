var mysuper = x => super[x]();
WScript.Echo("statements before super reference executed as expected");
mysuper();
WScript.Echo("ERROR:statements after super reference should not be executed");
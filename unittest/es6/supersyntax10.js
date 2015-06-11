var bar = function () { (y => super())(); }
WScript.Echo("statements before super reference executed as expected");
bar();
WScript.Echo("ERROR:statements after super reference should not be executed");
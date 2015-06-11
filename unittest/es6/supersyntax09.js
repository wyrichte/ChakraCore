function foo() { (x => super())(); }
WScript.Echo("statements before super reference executed as expected");
foo();
WScript.Echo("ERROR:statements after super reference should not be executed");
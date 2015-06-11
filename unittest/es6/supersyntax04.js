WScript.Echo("statements before 'super' reference executed as expected");
super['prop'];
WScript.Echo("ERROR:statements after 'super' reference should not be executed");
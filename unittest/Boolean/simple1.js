var f = false;
var t = true;

if(!f)      WScript.Echo("test 1");
if(!!!f)    WScript.Echo("test 2");
if(t)       WScript.Echo("test 3");
if(!!t)     WScript.Echo("test 4");

var obj = {}
var a = new Proxy(obj,{});
var m = new Array();
a; /**bp:evaluate('a',2, LOCALS_TYPE);evaluate('m',2, LOCALS_TYPE);**/
WScript.Echo("PASS");


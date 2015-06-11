var ab = new ArrayBuffer(1024);
var ta = new Uint8Array(ab);
WScript.Echo("Pass"); /**bp:evaluate('ab', 0, LOCALS_TYPE),evaluate('ta', 0, LOCALS_TYPE)**/
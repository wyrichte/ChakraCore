var arr = new Array;
arr.foobar = "foobar";
Object.defineProperty(Array.prototype, "4",{get : function() { return 30;}}); 
arr./**ml:foobar**/;

Number.prototype.foobar1 = "number foobar";
var a = new Int8Array(0);
var k = a.length;
k./**ml:foobar1**/;


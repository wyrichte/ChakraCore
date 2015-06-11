function write(v) { WScript.Echo(v); }

Object.prototype.toString = function() { return "toString() Overwritten"; }

var o = new Object();

//
// Currently toString is not looked up and hence not called.
// Commenting for now
//
write(o);

function write(v) { WScript.Echo(v + ""); }

// Win OOB Bug 1150770

Error.x = 10;
write(RangeError.x === 10);
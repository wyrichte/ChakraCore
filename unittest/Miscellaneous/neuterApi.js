var a = new Uint8Array(8);
WScript.Echo(a.byteLength);
WScript.Echo(Debug.detachAndFreeObject(a));
try {
	if (a.byteLength === 'undefined' || a.byteLength !== 0) {
		WScript.Echo('ArrayBuffer.byteLength should be 0 since it is detached.');
	}
} catch (ex) 
{
	WScript.Echo("Not expected to throw while trying to access ArrayBuffer.prototype.byteLength of detached buffer." + a.byteLength);
}

var a = new Uint8Array(8);
WScript.Echo(a.byteLength);
WScript.Echo(Debug.detachAndFreeObject(a.buffer));
try {
	if (a.byteLength === 'undefined' || a.byteLength !== 0) {
		WScript.Echo('ArrayBuffer.byteLength should be 0 since it is detached.');
	}
} catch (ex) 
{
	WScript.Echo("Not expected to throw while trying to access ArrayBuffer.prototype.byteLength of detached buffer." + a.byteLength);
}

var size = 1024 * 1024 * 256; //256mb
for (var i = 0; i < 100; i++) { // 25 GB
    var a = new Uint8Array(size);
    WScript.Echo(Debug.detachAndFreeObject(a.buffer));
}
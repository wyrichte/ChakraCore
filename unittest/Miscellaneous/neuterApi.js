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

// Testing detaching buffer with 0 length
var b = new Uint8Array(0);
Debug.detachAndFreeObject(b);

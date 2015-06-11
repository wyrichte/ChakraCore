// Note: you have to run this in jshost.exe, would not work with jc.exe
// VBArray is "function" for IE and is "undefined" (deprecated) in WWA
WScript.Echo(typeof VBArray);

if (typeof VBArray == "function")
{
WScript.Echo(VBArray.prototype.constructor);
WScript.Echo(VBArray.prototype.lbound);
WScript.Echo(VBArray.prototype.ubound);
WScript.Echo(VBArray.prototype.dimensions);
WScript.Echo(VBArray.prototype.getItem);
WScript.Echo(VBArray.prototype.toArray);
}

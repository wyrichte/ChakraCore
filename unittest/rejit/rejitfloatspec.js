function grow(a) {
  return 1.1 * a;
}

var a = 1;
var b = "hello";

WScript.Echo(grow(a));
WScript.Echo(grow(a));
WScript.Echo(grow(a));
WScript.Echo("    Initial JIT");
WScript.Echo(grow(b));

WScript.Echo("    Schedule instrumented reJIT");
WScript.Echo(grow(b));
WScript.Echo("    Instrumented reJIT");
WScript.Echo(grow(b));

WScript.Echo(grow(b));
WScript.Echo(grow(b));
WScript.Echo(grow(b));
WScript.Echo(grow(b));
WScript.Echo("    Schedule reJIT here, no float type spec");
WScript.Echo(grow(b));

WScript.Echo("    reJIT, no float type spec");
WScript.Echo(grow(b));

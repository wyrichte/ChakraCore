// Put a CheckThis scenario through its bailout and re-jit paces.

function caller() {
    return (function() {
        return this;
    })();
}

WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo("    Initial JIT");
WScript.Echo(caller());

WScript.Echo("    Schedule instrumented reJIT");
WScript.Echo(caller());

WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo("    Instrumented reJIT");
WScript.Echo(caller());

WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo("    Schedule reJIT here, no CheckThis");
WScript.Echo(caller());

WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo(caller());
WScript.Echo("    reJIT, no CheckThis");
WScript.Echo(caller());

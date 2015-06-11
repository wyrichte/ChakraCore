// Confirm that we can continue executing function calls and loop bodies when we fail
// to install jitted code after the native code gen job has succeeded. (Written to run
// with /mic:2 /lic:1 /on:failnativecodeinstall.)

var x = 0;
var y;

try {
    try {
        x++;
        // Interpret f, throw on jitting of loop body
        f();
    }
    catch (e) {
        WScript.Echo('caught call ' + x++);
        // Interpret f, throw on jitting of loop body
        f();
    }
}
catch (e) {
    WScript.Echo('caught call ' + x);
    try {
        try {
            x++;
            // Throw trying to jit function body
            f();
        }
        catch (e) {
            WScript.Echo('caught call ' + x++);
            // Throw trying to jit function body
            f();
        }
    }
    catch (e) {
        WScript.Echo('done');
    }
}

function f() {
    WScript.Echo('call ' + x);
    while (1) {
        y++;
    }
}


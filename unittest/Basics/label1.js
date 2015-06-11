function f() {
    // Label in parenthesis is bad syntax, but we allow it. Verify consistency in deferred parsing.
    (a):
        var i = 0;
}

f();

WScript.Echo("pass");

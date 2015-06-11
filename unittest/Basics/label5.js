function f() {
    // Bad label, verify consistency in deferred parsing
    a[0]:
        var i = 0;
}

f();

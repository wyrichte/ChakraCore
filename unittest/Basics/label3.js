function f() {
    // Bad label, verify consistency in deferred parsing
    a--:
        var i = 0;
}

f();

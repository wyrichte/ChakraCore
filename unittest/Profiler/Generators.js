function test() {
    function* gf() { }
    gf().next();
    gf().return();
    try {
        gf().throw();
    } catch (e) {
    }
}

WScript.StartProfiling(test);

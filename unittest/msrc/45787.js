(function() {
    const b = function (i) {
        if (i === 99) delete b.foo;
    };
    b.foo = 3;

    for (let z = 0; z < 100; ++z) {
        b(z);
    }
})();
WScript.Echo("PASSED");
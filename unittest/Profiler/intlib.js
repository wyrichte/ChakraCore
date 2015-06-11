function foo() {
    function bar() { return 3; }

    var array = [
        { toString: function () { return bar(); } },
        5
    ];
    var collator = new Intl.Collator();
    array.sort(collator.compare);
}

WScript.StartProfiling(foo);

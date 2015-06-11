/**exception(resume_ignore):stack()**/

function runTest(test) {
    function trimPath(line) {
        return line && line.replace(/\(.+unittest.Debugger./ig, "(");
    }

    // Error.stack
    try {
        test();
    } catch (e) {
        WScript.Echo(trimPath(e.stack));
    }

    // Unhandled exception
    test();
}

function callBadFormat() {
    var formatter = new Intl.NumberFormat("INVALID CURRENCY CODE");
}

// Intl.Collator.prototype.compare
// Array.prototype.sort
runTest(function () {
    var count = 0; // Only run once
    var array = [
        { toString: function () { if (count++ == 0) { callBadFormat(); } } },
        5
    ];
    var collator = new Intl.Collator();
    array.sort(collator.compare);
});

var bad_locale = { toString: function () { throw new Error("Intentional throw"); } };
var bad_locales = [bad_locale];

// Intl.Collator
// Intl.Collator.supportedLocalesOf
// Intl.DateTimeFormat
// Intl.DateTimeFormat.supportedLocalesOf
// Intl.DateTimeFormat.prototype.format
// Intl.NumberFormat
// Intl.NumberFormat.supportedLocalesOf
// Intl.NumberFormat.prototype.format
[Intl.Collator, Intl.DateTimeFormat, Intl.NumberFormat].forEach(function (obj) {
    runTest(function () {
        new obj(bad_locales);
    });

    runTest(function () {
        obj.supportedLocalesOf(bad_locales);
    });

    if (obj.prototype.format) {
        runTest(function () {
            new obj().format(bad_locale);
        });
    }
});

// Date.prototype.toLocaleString
// Date.prototype.toLocaleDateString
// Date.prototype.toLocaleTimeString
// Number.prototype.toLocaleString
// String.prototype.localeCompare
(function () {
    var d = new Date();
    [d.toLocaleString, d.toLocaleDateString, d.toLocaleTimeString].forEach(function (f) {
        runTest(function () {
            f.apply(d, [bad_locales]);
        });
    });

    runTest(function () {
        var n = 0;
        n.toLocaleString(bad_locales);
    });

    runTest(function () {
        "abc".localeCompare("def", bad_locales);
    });
})();


(function () {
    // Array
    var arr = [1];
    [arr.every, arr.some, arr.forEach, arr.map, arr.filter, arr.reduce, arr.reduceRight].forEach(function (f) {
        runTest(function () {
            f.apply(arr, [callBadFormat, 0]);
        });
    });

    // JSON
    runTest(function () {
        JSON.parse("{}", callBadFormat);
    });
    runTest(function () {
        JSON.stringify({}, callBadFormat);
    });

    // Map, Set
    var m = new Map(); m.set(1, 1);
    var s = new Set(); s.add(1);
    [m, s].forEach(function (obj) {
        runTest(function () {
            obj.forEach(callBadFormat);
        });
    });

    // String
    runTest(function () {
        "abcd".replace("ab", callBadFormat);
    });
})();

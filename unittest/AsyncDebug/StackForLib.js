function runTest(test) {
    test();
}

function callBadFormat() {
    Debug.msTraceAsyncOperationStarting("opId1", 1);
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

var bad_locale = { toString: function () { 
    Debug.msTraceAsyncOperationStarting("opId2", 2);
} };
var bad_locales = [bad_locale];

// Intl.Collator
// Intl.Collator.supportedLocalesOf
// Intl.DateTimeFormat
// Intl.DateTimeFormat.supportedLocalesOf
// Intl.DateTimeFormat.prototype.format
// Intl.NumberFormat
// Intl.NumberFormat.supportedLocalesOf
// Intl.NumberFormat.prototype.format
[Intl.DateTimeFormat].forEach(function (obj) {
    runTest(function () {
        return bad_locales + "";
    });

 });

 (function () {
    var d = new Date();
    [d.toLocaleString, d.toLocaleDateString, d.toLocaleTimeString].forEach(function (f) {
        runTest(function () {
            return bad_locales + "";
        });
    });

})();

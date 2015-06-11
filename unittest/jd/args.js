//
// test arguments
//

(function () {
    function f(x0, x1, x2, x3, x4, x5, x6, x7) {
        for (var i = 0; i < arguments.length; i += 3) {
            delete arguments[i];
        }
        /**bp:locals(1, LOCALS_TYPE)**/
    }

    var s = "f.apply({},[";
    for (var i = 0; i < 65; i++) {
        s += i + ",";
    }
    s += i + "])";
    eval(s);
})();

WScript.Echo("pass");
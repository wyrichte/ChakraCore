//
// test "this"
//

function f1() {
    ;
    /**bp:locals(1)**/
}

function f2() {
    "use strict";
    ;
    /**bp:locals(1)**/
}

[null, undefined, true, 123, "String Value", new Boolean(false), new Number(45), new String("String Object"), [0,1], { obj: "member" }, this]
    .forEach(function (x) {
        f1.apply(x, [x]);
        f2.apply(x, [x, "strict mode"]);
    });

//
// Side test real "arguments". (Above tests all have fake arguments.)
//
// Note that in strict mode, in-proc debugger returns 2 Error objects, while hybrid debugging returns 2 error strings for
// callee and caller. If we expand more than 1 level below, or if we start to display their "<large string>" content, we can't share baseline.
//
(function (){
    function f1() {
        arguments;
        /**bp:locals(1)**/
    }

    function f2() {
        "use strict";
        arguments;
        /**bp:locals(1)**/
    }

    f1.apply({});
    f2.apply({});
})();

WScript.Echo("pass");

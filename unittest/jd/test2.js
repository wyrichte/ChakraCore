// test function name

var foo = function () {
    Debug.sourceDebugBreak();
};

function func() {
    foo();
}

var constructed = new Function("func();");

function bar() {
    (function () {
        eval("constructed();");
    })();
}

bar();

(function () {
    var j = "Test Anonymous function";
    WScript.Echo(j);
    return j;
})();

function topFunc() {
    function F1() {
        var a = 10; var b = 20; var c = 30;
        WScript.Echo("In function f1");
        return a + b;
    }
    F1();

    function F2() {
        var a = new Array;
        for (var i = 0; i < 1000000; i++) {
            a.push(i * 10);
        }

        var sub = -10;
        for (var i = 0; i < 1000000; i++) {
            a[i] -= sub;
        }
        WScript.Echo("in F2, value : " + a[50000]);
        return a.length;
    }

    F2();

    (function () {
        var j = "Anonymous function2";
        WScript.Echo(j);
        return j;
    })();

    eval("function f3() { var m = 20; var m1 = new Date; var m2 = new Array(1000); m1 = (new Date()) - m1; }")
    f3();

    var add1 = new Function("x", "y", "return(x+y)");
    WScript.Echo(add1(20, 4));

    eval("function f31() { var m = 20; var m1 = new Date; var m2 = new Array(2000); m1 = (new Date()) - m1; }\nf31()")
    f31();

    function F4(a, b) {
        var a1 = 10;
        a1++;
        var f6 = function () {
            var b1 = 10;
            return b1;
        }
        f6();
        return a1;
    }
    F4();
}

topFunc();
CollectGarbage();

function topFunc1() {
    WScript.Echo("In topFunc1");
}
topFunc1();
CollectGarbage();

WScript.StartProfiling(topFunc);

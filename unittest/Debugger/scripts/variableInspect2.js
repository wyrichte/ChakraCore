function clickme() {
    var obj = {};
    function f1() {
        this.inf1 = 1;
    }

    function f2() {
        this.inf2 = 2;
    }

    var a1 = new f1();

    f2.prototype = new f1();

    var a2 = new f2();

    a2.x = 10;
    a2.foo = function () {}
    return a2.inf2;
}
clickme()

function clickme2() {
    var y = "hello";
    var x = 20;
    function f(b) {
        var y = 10;
        y++;
        function bar(c) {
            var z = 11;
            z++;
        }
        bar(23);
    }
    f(1);
}

clickme2();

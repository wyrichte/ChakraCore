var stuff = [];
function range(min, max) {
    var a = [];
    for (; min < max; min++) {
        a.push("my string:" + min);
    }
    return a;
}

function makeBig() {

    var f = function (a) {
        return a;
    }

    var dataOne = range(0, 3);
    var g = f.bind(null, dataOne);
    g[11111] = 11111;
    g[22222] = 22222;
    g.abacaba = "abacaba";
    g["xxxxxxx"] = "yyyyyy";
    stuff.push(g);

    var dataTwo = range(0, 6);
    stuff.push(function () {
        return f(dataTwo);
    });

    var o = { x: 1 };
    var args = ["p1", "p2", o];
    stuff.boundFunctionWithThisAndArgs =  f.bind(o, args);

    var p = { y : 2 };
    stuff.boundFunctionWithThisAndNoArgs =  f.bind(p);
}

function Run() {
    makeBig();
    CollectGarbage();

    var r1 = stuff[0]();
    var r2 = stuff[1]();

    WScript.Echo(r1[10]);

    Debug.dumpHeap(stuff);
};

Run();
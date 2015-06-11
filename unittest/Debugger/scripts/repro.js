var i = 1+2+3;
for (var j = 0; j < 3; j++) {
    k = 4+5 + i + j;
}

var array = [];

function f1(x) {
    return x();
}

function f2() {
    var i = 3;
    var j = 0;
    while (i--) {
        array.push(f1(function () { return i; })); j++;
    }

    return array.length;
}

f2();


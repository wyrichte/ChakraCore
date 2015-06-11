

var array = [];
var obj = {};

function f1(x) {
    return x();
}

var i = 3;
while(i--) {
    array.push(f1(function (){return i;}));
}

function f2() {
    var i =4;
    var j = 2;
    return j;
}

function f3() {
    f4();
    return array.length;
}

function f4() {
    var val = f2();
    val = val.toString();
    return val;
}

function f5() {
    throw 1;
}

try {
   f5();
}
catch (e) {
    f3();
}




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
    debugger;
    var j = 2;
    return;
}

function f3() {
    f4();
}

function f4() {
    throw 23;
}

function f5() {
    f2();
    debugger;
}

try {
   f3();
}
catch(e) {
   try {
      f5();
   }
   finally{
      f5();
   }
}
throw 4;

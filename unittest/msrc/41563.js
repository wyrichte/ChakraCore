var b = undefined;
function foo() {
var o = {length: 5};
for (var i = 0; o; ) {
for (var j = 0; o.length; ) {
o = b + 45;
}
}
}
foo();
foo();
b = -45;
foo();


function bar(length, value) {
    var o = {};
    o.length = length;
    var arr = new Array(length);
    for (var i = 0; i < o.length; ++i)
        o[i] = value;
    for (var i = 0; i < o.length; ++i) {
        for (var j = 0; j < o.length; ++j)
            o = function(a) { return a; }[i] += o[j];
    }
    return o;
}
bar(5, 42);
bar(5, 42);
bar(5, 42);
print("passed");
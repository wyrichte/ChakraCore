function* f() {
    if (this == 0x1234) {
        print("PASSED");
    } else {
        print("FAILED");
    }
    if (arguments[0] == 0x5678) {
        print("PASSED");
    } else {
        print("FAILED");
    }
}

let g;
f.__defineGetter__('length', function () {
g = this; // g == "scriptFunction"
});


f.length;

var v = g.call(0x1234, 0x5678).next();
if (v.done) {
    print("PASSED");
} else {
    print("FAILED");
}

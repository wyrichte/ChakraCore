print('start');
for (var ii = 0; ii < 10; ii++) {
    try {
        f()
    } catch (FUNCALL) {print(FUNCALL)}
}

print('crash');
f.call( {}, {}, WScript.Echo, {})

function f(i0, i1, i2) {
    i1(print((function () {
        for (var x = 0; x < 10; x++) {
            (i0 = [i2]);
        }
        for (var x = 10; x > -1; x--) {
            i0[--x] = (20)
        }
    })()))

    arguments.shift(); // ???
}

print('finish');

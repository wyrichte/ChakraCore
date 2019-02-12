function opt(a, b) {
    a.a = 1
    b.pop()
    a.a = 1
    a.c = 1
}

for (var i = 0; i < 2; i++) {
    var a = { "length": 0x100000001, "pop": [].pop, "a": 1, "b": 1 }
    a[0x100000000] = 1

    var b = {}
    b.pop = [].pop

    opt(a, b)
}

var a = { "length": 0x100000001, "pop": [].pop, "a": 1, "b": 1 }
a[0x100000000] = 1
opt(a, a)
if (a.a === 1)
    WScript.Echo('pass');


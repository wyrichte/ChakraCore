function opt(n) {
    var cls = (class { });
    for (let i = 0; i < n; i++) {
        new cls();  // use 0x000100000000dead as pointer.
        cls = 0xdead
    }
}

for (let i = 0; i < 10; i++) {
    opt(1);
}

try {
    opt(2);
}
catch(e) {
    WScript.Echo('pass');
}

function main(o) {
    o.x = 0;
    var i = o;
    for (var j = 0; j < 3; j++) {
        var a = o.valueOf();
        o = i;
        i = 1.1;
    }
}

for (var i = 0; i < 160; i++) {
    main({});
}

WScript.Echo('pass');

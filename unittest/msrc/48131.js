function cons() {
}

function opt(o, value) {
    o.b = 1;
    new cons();
    o.a = value;
}

function main() {

    for (let i = 0; i < 2; i++) {
        opt({ a: 1, b: 2 }, {});
    }

    let o = { a: 1, b: 2 };
    cons.prototype = o;
    opt(o, 4660);
    if (o.a === 4660)
        WScript.Echo('pass');
}

main();



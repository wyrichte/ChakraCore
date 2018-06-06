function opt() {
    let obj = new Number(2.3023e-320);
    for (let i = 0; i < 1; i++) {
        obj.x = 1;
        obj = +obj;
        obj.x = 1;
    }
}

function main() {
    for (let i = 0; i < 1000; i++) {
        opt();
    }
}

main();

function opt2() {
    let obj = '2.3023e-320';
    for (let i = 0; i < 1; i++) {
        obj.x = 1;
        obj = +obj;
        obj.x = 1;
    }
}

function main2() {
    for (let i = 0; i < 1000; i++) {
        opt2();
    }
}

main2();
print("passed");
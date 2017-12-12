function opt(i) {
    let tmp = [];
    tmp[i] = tmp;
    return tmp[i];
}

function main() {
    for (let i = 0; i < 0x1000; i++) {
        opt(i);
    }
    let result = opt(0);
    if(Array.isArray(result) && result.length === 1 && result[0] === result) {
        print("Pass")
    } else {
        print("Fail")
    }
}
main();

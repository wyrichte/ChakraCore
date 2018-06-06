function opt(obj) {
    for (let i in obj.inlinee.call({})) {
    }

    for (let i in obj.inlinee.call({})) {
    }
}

function main() {
    let obj = {
        inlinee: function () {
        }
    };

    opt(obj);
    opt(obj);
    opt(obj);
}

main();
print("pass");
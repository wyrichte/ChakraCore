function opt(arr, s) {
    arr[0] = 1.1;

    if (s !== null) {
        let tmp = 'a'.localeCompare(s);
    }

    arr[0] = 2.3023e-320;
}

function main() {
    let arr = [1.1];

    for (let i = 0; i < 100; i++) {
        'a'.localeCompare('x', []); // Optimize the JavaScript localeCompare

        opt(arr, null); // for profiling all instructions in opt.

        try {
            opt(arr, {
                toString: () => {
                    throw 1; // Don't profile "if (locales === undefined && options === undefined) {"
                }
            });
        } catch (e) {

        }
    }

    opt(arr, {
        toString: () => {
            // Called twice
            arr[0] = {};
        }
    });

    if (arr[0] === 2.3023e-320) {
        print("pass");
    } else {
        print("fail");
    }
}

main();

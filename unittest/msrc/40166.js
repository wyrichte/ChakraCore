function f1() {
}
f1();

function f2() {
    ((a = function () { }, b = () => 1) => {
        print(b() === 1 ? "PASSED" : "FAILED");
    })();
};
f2();

function test() {
    var kcngmm = function () {
    };
    var u3056 = function () {
    };
    with (u3056.__proto__ = Object.seal.bind(undefined, /x/g)) {
        (function () {
            return '';
        }());
    }
    try {
        try {
            u3056.caller = kcngmm;
        } catch (e) {
        }
        try {
            ;
        } catch (e) {
        }
    } catch (e) {
    }
}
test();
WScript.Echo("Pass");

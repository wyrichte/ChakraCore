
function test8() {
    var test = function () {  };
    var p = new Proxy(test, {
        get: function (target, prop) {
            foo('get : ' + prop);   // reference error.
            return Reflect.get(target, prop);
        }
    })
    p./**ml:bind**/;
}
test8();

function test11() {
    var trap = {
        getPrototypeOf: function (target) {
            print('getPrototypeOf trap'); // reference error here.
            return { a: "a" };
        },

    }
    function test(a, b) {
    }

    var p = new Proxy(test, trap);
    var x = p./**ml:-**/bind({}, 1);
}

test11();

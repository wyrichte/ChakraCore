
function test0() {
    print('test0 : Object.keys with symbols');
    var sym = Symbol();
    var o = {};
    o[sym] = "blah";

    var p = new Proxy(o, {});
    WScript.Echo(Object.keys(p).length);
}

function test1() {
    print('test1: Object.prototype.propertyIsEnumerable');
    var sym = Symbol();
    var o = {};
    Object.defineProperty(o, sym, { value: 5, enumerable: true });
    print(o.propertyIsEnumerable(sym));
}

function test2() {
    print('test2: Object.getOwnPropertyDescriptor');
    var desc = { value: new Proxy({}, {}), writable: true, enumerable: true, configurable: true};

    var traps =
    {
        getOwnPropertyDescriptor: function () { WScript.Echo("getown"); return desc; }
    };

    var p = new Proxy({}, traps);
    WScript.Echo(Object.getOwnPropertyDescriptor(p).value);

    traps.getOwnPropertyDescriptor = function () {
        WScript.Echo("proxy getown");
        desc.get = function () { return 5; };
        return new Proxy(desc, {  });
    }

    try {
        Object.getOwnPropertyDescriptor(p);
        print('Expected to throw TypeError');
    } catch (e) {
        if (e instanceof TypeError) {
            if (e.message !== 'Property cannot have both accessors and a value') {
                print('FAIL');
            }
        } else {
            print('FAIL');
        }
    }
}

function test3(){
    var traps = {
        has: function (target, prop) {
            print('has trap for prop :' + prop);
            return Reflect.has(target, prop);
        },

        getOwnPropertyDescriptor: function (target, prop) {
            print('getOwnPropertyDescriptor trap for prop: ' + prop);
            return new Proxy(desc, traps);
        }
    };

    var desc = { value: 1, writable: true, configurable : true };
    desc.a = 1;
    var p = new Proxy(desc, traps);
    Object.getOwnPropertyDescriptor(p,"a");
}

function test4() {
    var keys = ["a"];
    var traps =
    {
        ownKeys : function() { WScript.Echo("plain key trap!"); return keys; },
        getOwnPropertyDescriptor: function (target, prop) {
            WScript.Echo("getOwn");
            return { enumerable: true, configurable: true }
        }
    };
    var p = new Proxy({}, traps);
    WScript.Echo(Object.keys(p).length);
    traps.ownKeys = function (target, prop)
    {
        WScript.Echo("proxy key trap!");
        return new Proxy(keys, {});
    }
    WScript.Echo(Object.keys(p).length);
}

function test5() {
    var keys = ["a"];
    var traps =
    {
        ownKeys: function () { WScript.Echo("plain key trap!"); return keys; },
        getOwnPropertyDescriptor: function (target, prop) {
            WScript.Echo("getOwn :" + prop);
            return { enumerable: true, configurable: true }
        }
    };
    var p = new Proxy({}, traps);
    //WScript.Echo(Object.keys(p).length);
    traps.ownKeys = function (target, prop) {
        WScript.Echo("proxy key trap!");
        return { 0: "a",  2: "3", length : 2 }
    }
    WScript.Echo(Object.keys(p).length);
}

function test6() {
    var arr = [1, 2, 3];
    Math.max.apply(null, new Proxy(arr, {
        get: function (target, prop) {
            print('get trap : ' + prop);
            if (prop == 'length') {
                return target.length;
            }
        }
    }));
}

function test7() {
    var traps = {
        get: function (target, prop) {
            print('get trap :' + prop);
            return Reflect.get(target, prop);
        },
        ownKeys: function (target) {
            print('ownKeys trap : ');
            return Reflect.ownKeys(target);
        },
        getOwnPropertyDescriptor: function (target, prop) {
            print('getOwnPropertyDescriptor trap : ' + prop.toString());
            return Reflect.getOwnPropertyDescriptor(target, prop);
        }
    };

    var proto = { inherited: "blah" };
    var props = Object.create(proto);
    var sym1 = Symbol();

    Object.defineProperty(props, "a", { value: 5 });
    Object.defineProperty(props, "b", { value: 5 });
    Object.defineProperty(props, sym1, { value: 5 });

    var proxy_props = new Proxy(props, traps)
    var o1 = Object.create(proto, proxy_props);
    var o2 = Object.defineProperties({}, proxy_props);
}

function test8() {
    var test = function () { print('test') };
    var p = new Proxy(test, {
        has: function (target, prop) {
            print('has');
        },

        get: function (target, prop) {
            print('get : ' + prop);
            return Reflect.get(target, prop);
        }
    })
    p.bind({});
}

function test9() {
    var test = function () { print('test'); }
    var p = new Proxy(test, {
        apply: function (target) {
            print('apply');
        }
    });
    p.call();
}


// Function.bind with proxy
function test10() {
    function test() { print('test called'); }
    var p = new Proxy(test, {});
    var x = p.bind({}, 1, 2);
    var proxy_x = new Proxy(x, {});
    print(x.name);
    print(proxy_x.name);
    print(p.name);
    p();
    x();
    proxy_x();
}

function test11() {
    var trap = {
        get: function (target, property) {
            print('get trap: ' + property);
            return Reflect.get(target, property);
        },

        getPrototypeOf: function (target) {
            print('getPrototypeOf trap');
            return { a: "a" };
        },

        getOwnPropertyDescriptor: function (target, property) {
            print('getOwnPropertyDescriptor trap: ' + property);
            return Reflect.getOwnPropertyDescriptor(target, property);
        }
    }
    function test(a, b) {
    }

    var t = test.bind({}, 1);
    var p = new Proxy(test, trap);
    var x = p.bind({}, 1);
    var proxy_x = new Proxy(x, {});
    print(Object.getPrototypeOf(proxy_x).a === "a");
    print(Object.getPrototypeOf(x).a === "a");
}

function test12() {
    var o = {};
    Object.defineProperty(o, "A", { get: function () { return 5; }, set: function (val) { } });
    var p = new Proxy(o, {
        getOwnPropertyDescriptor: function (target, property) {
            print('getOwnPropertyDescriptor trap :' + property);
            return Reflect.getOwnPropertyDescriptor(target, property);
        },
        get: function (target, property) {
            print('get trap :' + property);
            return Reflect.get(target, property);
        }
    })

    p.__lookupGetter__("A");
    p.__lookupSetter__("A");
}

function test13() {
    function Foo() { }

    Object.defineProperty(Foo, 'length', { value: 123, enumerable: true, configurable: false });
    print(Foo.length);

    var x = new Proxy(Foo, {
        ownKeys: function (target) {
            print("my proxy ownKeys");
            return Reflect.ownKeys(target);
        }
    });
    print(Object.keys(x));
}


test0();
test1();
test2();
test3();
test4(); 
test5();
test6();
test7();
test8();
test9();
test10();
test11();
test12();
test13();
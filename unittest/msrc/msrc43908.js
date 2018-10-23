(function() {
    // This is original POC converted to run from jshost - but it does not repro.
    
    var handler = {};
    var oo = new Proxy({}, handler);; 

    var sc0 = WScript.LoadScript(`
    function setProto(oo, handler, callback) {
        handler.getPrototypeOf = function () {
            print('getPrototypeOf parent');
            callback();
            return {};
        };
    }

    `, 'samethread');

    var a = Debug.createTypedObject(2000, "mytype", 32);
    var b = Debug.addFTLProperty(a, "test", 1, oo);

    function opt(a1, arr) {
        arr[0] = 1.1;
        let res = a1.test;
        arr[0] = 2.3023e-320;
        return res;
    }

    let arr = [1.1];

    // get to the jit
    for (let i = 0; i < 10000; i++) {
        opt(a, arr);
    }
    
    // opt is jitted.

    function ff() {
        arr[0] = {};
    }

    sc0.setProto(oo, handler, ff);
    opt(a, arr);
    arr;
})();

(function() {
    // This test is exhibitting that proxy's getPrototypeOf will not be called when we just marshal the proxy object.
    
    var sc0 = WScript.LoadScript(`
    var obj = {};
    var handler = {};
    handler.getPrototypeOf = function () {
        WScript.Echo('getPrototypeOf proxy');
        return {};
    };
    obj.p = new Proxy({}, handler);
    `, "samethread");

    var k = sc0.obj.p;
    // getprototypeof should not be called.
    print('after sc0.obj.p'); 
    print(k);
    print(k.__proto__);
})();

(function() {
    // This test is exhibitting that proxy's getPrototypeOf will not be called when the proxy is on the prototype chain
    // of the object which we are marshalling.
    var called = 0;
    var proxy = new Proxy({}, {getPrototypeOf : function() {
      called++;
      return {};
    }});
    
    var obj = {};
    obj.__proto__ = proxy;
    called = 0;
    
    var sc0 = WScript.LoadScript(`function foo() {
        print(obj);
        print(obj.__proto__);
        print(obj.__proto__.__proto__);
    }
    `, "samethread");
    sc0.obj = obj;
    print(called == 0); // above call should not invoke the proxy's getPrototypeOf
    sc0.foo();
    print(called == 1); // call to foo should invoke the proxy's getPrototypeOf
})();

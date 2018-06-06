function zee(fn) {
    try {
        fn();
    } catch(ex) {
        print('Exception thrown : ' + ex.message);
    }
}

(function() {
    print('new Car - test (MSRC 39098)');
    var a = '0x42424242,'.repeat(0xFFFF-2); 
    var b = "function Car(){} var car = new Car(a,"+a+"a);";
    zee(function() { eval(b) });
})();

(function () {
    print('foo() call - test');
    function foo() {};
    function test1(count) {
        print('count : ' + count);
        var a = 1;
        var cmd = 'foo(a, ';
        for (var i = 0; i < count; i++) {
            cmd += 'a, ';
        }
        cmd += 'a );';
        eval(cmd);
    }
    
    for(var i = 2**16-15; i < 2**16 + 15; i++) {
        zee(function() { test1(i) });
    }

})();

(function () {
    print('new foo() call - test');
    function foo() {};
    function test1(count) {
        print('count : ' + count);
        var a = 1;
        var cmd = 'new foo(a, ';
        for (var i = 0; i < count; i++) {
            cmd += 'a, ';
        }
        cmd += 'a );';
        eval(cmd);
    }
    
    for(var i = 2**16-15; i < 2**16 + 15; i++) {
        zee(function() { test1(i) });
    }
})();

(function () {
    print('eval() call - test');
    function foo() {};
    function test1(count) {
        print('count : ' + count);
        var a = 1;
        var cmd = 'eval(a, ';
        for (var i = 0; i < count; i++) {
            cmd += 'a, ';
        }
        cmd += 'a );';
        eval(cmd);
    }
    
    for(var i = 2**16-15; i < 2**16 + 15; i++) {
        zee(function() { test1(i) });
    }
})();

(function() {
    print('super call test');
    class A {
        constructor() {
        }
    }
    class B extends A {
        constructor(count) {
            print('count : ' +count);
            var a = 1;
            var cmd = 'super(a, ';
            for (var i = 0; i < count; i++) {
                cmd += 'a, ';
            }
            cmd += 'a);';
            eval(cmd);
        }
    }
    
    for(var i = 2**16-15; i < 2**16 + 15; i++) {
        zee(function() { new B(i); });
    }
})();

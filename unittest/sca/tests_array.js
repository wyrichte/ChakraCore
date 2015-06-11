//
// SCA tests for arrays
//
var tests_array = (function(){
    var tests = [
        [],
        [null],
        [undefined],
        [0],
        [,,],
        [0, null, undefined, , 3, "4", {a:5, b:[6,7,8]}],
    ];

    // sparse array
    var root = [0,1,2];
    root.length = 100;
    tests.push(root);

    // sparse array
    var root = [];
    root[0] = 1;
    root[10000] = 10000;
    tests.push(root);

    // sparse array with boundary index
    var root = new Array(4294967295);
    root[0] = 2;
    root[4294967294] = 4294967294;
    root[4294967295] = 4294967295;
    root[4294967296] = 4294967296;
    tests.push(root);

    // nested + cycles
    var root = [{}, {}];
    root[0].a0 = root;
    root[0].a1 = new String("a1");
    root[1].b = [];
    root[1].b[0] = root;
    root[1].b[1] = root[0].a0;
    root[1].b[2] = root[0].a1;
    root[1].b[3] = root[1].b;
    tests.push(root);

    // an array containing primitives
    WScript.LoadScriptFile("tests_primitive.js");
    var root = [];
    tests_primitive.forEach(function(test){
        root.push(getTestObject(test));
    });
    tests.push(root);

    // an array containing builtins
    WScript.LoadScriptFile("tests_builtin.js");
    var root = [];
    tests_builtin.forEach(function(test){
        root.push(getTestObject(test));
    });
    tests.push(root);

    // -------------------------------------------------------
    // Now bulk add empty lookups. We don't lookup into arrays.
    tests = tests.map(function(arr) {
        var test = {};
        test.root = arr;
        test.lookups = [];
        return test;
    });

    // Add a named property to each array
    tests = tests.concat(tests.map(function(test){
        var t = {};
        t.root = cloneArray(test.root);
        t.root.ab = 4096;
        t.lookups = cloneArray(test.lookups);
        t.lookups.push(["ab"]);
        return t;
    }));

    // For each test array, test an ES5Array copy
    tests = tests.concat(tests.map(function(test){
        var t = {};
        t.root = cloneArray(test.root);
        if (root.length > 0) {
            Object.defineProperty(root, 0, {
                writable: false
            });
        } else {
            Object.defineProperty(root, 0, {
                value: "Should NOT see this",
                enumerable: false
            });
        }
        t.lookups = test.lookups;
        return t;
    }));

    // root Object containing array
    var root = {a:[],b:[0,,2]};
    root.a.a1 = "a1";
    root.b.b1 = root.a;
    var test = {};
    test.root = root;
    test.lookups = [
        ["a"],
        ["b"],
        ["b", "b1", "a1"]
    ];
    tests.push(test);

    return tests;
})();

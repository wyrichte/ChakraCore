//
// SCA tests for Object
//
var tests_object = [
    {},

    {a:1, bb:"2"},

    {a:1, bb:"2", "":{}},

    {a:1, bb:"2", ccc:{"-":3, "d\0":4}, "-1":5, "123":"6", "":7},

    {
        get: function() {
            var root = {
                a: {},
            };
            root.b = root.a;
            return root;
        }
    },

    {
        get: function() {
            var root = {
                a: {},
            };
            root.b = root;
            return root;
        }
    },

    {
        get: function() {
            var root = {
                a: {
                    b: null,
                    c: undefined,
                    d: {}
                }
            };
            root.a.d.e = root;
            root.a.d.f = root.a;
            root.a.d.g = root.a.c;
            root.a.d.h = root.a;
            root.i = root.a.d;
            root.j = root.a.d;
            return root;
        }
    },

    {
        // Verify properties on prototype are ignored.
        get: function() {
            var proto = {
                prop: 8
            };
            var root = Object.create(proto, {});
            return root;
        }
    },

    {
        // Verify non-enumerable properties are ignored.
        get: function() {
            var root = {
                a: 9
            };
            Object.defineProperties(root, {
                b: {value: 10, enumerable: true},
                c: {value: 11, enumerable: false},
                d: {get: function() { return 12; }, enumerable: true},
                e: {get: function() { return 13; }, enumerable: false}
            });
            return root;
        }
    }
];

//
// Add simple objects containing only 1 property of a primitive or builtin.
//
WScript.LoadScriptFile("tests_primitive.js");
WScript.LoadScriptFile("tests_builtin.js");

(function(){
    var simple_objects = [];
    tests_primitive.concat(tests_builtin).forEach(function(test){
        var root = {};
        root.p = getTestObject(test);
        simple_objects.push(root);
    });
    tests_object = simple_objects.concat(tests_object);
})();

//
// Test SCA lookup API
//
WScript.LoadScriptFile("sca_lib.js");

WScript.LoadScriptFile("tests_object.js");
var testObjects = tests_primitive.concat(tests_builtin).concat(tests_object).concat([
    // An object containing primitive properties
    {
        get: function() {
            var root = {};
            addProperties(root, tests_primitive.map(getTestObject));
            return root;
        }
    },

    // An object containing builtin object properties
    {
        get: function() {
            var root = {};
            addProperties(root, tests_builtin.map(getTestObject));
            return root;
        }
    },

    // various property names
    {
        a: 1,
        bb: "2",
        "-": 3,
        "d\0": 4,
        "-1": 5,
        "123": "6",
        "": 7
    },

    // nested
    {
        root: {
            a:{
                a1:1,
                a2: {
                    a21: 2.1,
                    a22: {
                        a221: "a221",
                        a222: 2.22
                    },
                },
            },
            b: {
                b1:"b1",
                b2: {
                    b21: "b21"
                }
            }
        },
        lookups: [
            ["a", "a1"],
            ["a", "a2", "a21"],
            ["a", "a2", "a22", "a221"],
            ["a", "a2", "a22", "a222"],
            ["a", "a2", "a22", "a223"],
            ["b", "b1"],
            ["b", "b2", "b21"]
        ]
    },

    // cycles
    {
        get: function() {
            var root = {
                a: {
                    a1: "a1"
                },
                b: {
                    b1: {
                        b11: {
                            b111: {
                            }
                        }
                    }
                }
            };
            root.c = root.a;
            root.d = root;
            root.b.b1.b11.b111.b1111 = root.c;
            return root;
        },
        lookups: [
            ["a", "a1"],
            ["b", "b1", "b11", "b111", "b1111", "a1"],
            ["c", "a1"],
            ["d", "a", "a1"],
            ["d", "d", "d", "c", "a1"],
        ]
    },

    // host object
    {
        get: function(){
            return new ImageData({
                width: 12,
                height: 34,
                data: [1, 2, 3, 4]
            });
        }
    },

    // host object with cycle
    {
        get: function(){
            var img = new ImageData({
                width: 123,
                height: 3456,
                data: [2, 3, 4, 5, 6, 7, 8, 9], // PixelArray verifies length % 4 == 0
                type: "text/png",
                size: SCA.makeUint64(1, -1),
                compression: 0.4,
                lastModifiedDate: new Date("2011-11-15T13:44:00.000Z")
            });
            var root = {a:{a1:null},b:{b1:undefined}};
            root.a.a2 = img;
            root.b.b2 = img;
            root.b.b3 = img;
            return root;
        },
        lookups: [
            ["a", "a2"],
            ["a", "a2", "width"],
            ["b", "b2", "width"],
            ["b", "b2", "height"],
            ["b", "b3"],
            ["b", "b3", "data"],
            ["b", "b3", "type"],
            ["b", "b3", "size"],
            ["b", "b3", "compression"],
            ["b", "b3", "lastModifiedDate"]
        ]
    },
]);

// Lookup internal properties
testObjects = testObjects.concat((function () {
    var length_lookups = [
        ["length"],
        ["lengt"],
        ["lengtH"],
        ["length "],
        ["noprop", "length"],
        ["", "length"],
        ["length", ""],
        ["length", "length"],
        ["length", "abc", "length"],
    ];

    return [
        {
            root: {abc: 123},
            lookups: length_lookups
        },

        {
            root: { length: 123 },
            lookups: length_lookups
        },

        {
            root: /abc/,
            lookups: length_lookups
        },

        {
            root: [],
            lookups: length_lookups
        },

        {
            root: new Array(179),
            lookups: length_lookups
        },

        {
            root: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 1, 2, 3, 4, 5, 6, 7],
            lookups: length_lookups
        },

        {
            root: "hello",
            lookups: length_lookups
        },

        {
            root: new String("hello"),
            lookups: length_lookups
        },

        {
            root: /abcde/g,
            lookups: [
                ["source"],
                ["global"],
                ["ignoreCase"],
                ["multiline"],
                ["source", "length"],

                ["sourcE"],
                ["globaL"],
                ["ignorecase"],
                ["mulTiline"],
                ["source", "lEngth"],

                ["", "source"],
                ["", "global"],
                ["", "ignoreCase"],
                ["", "multiline"],
                ["", "source", "length"],

                ["source", ""],
                ["global", ""],
                ["ignoreCase", ""],
                ["multiline", ""],
                ["source", "length", ""],
                ["source", "", "length"],

                ["source", "source"],
                ["global", "global"],
                ["ignoreCase", "ignoreCase"],
                ["multiline", "multiline"],
                ["source", "length", "length"],
            ]
        },
    ];
})());

WScript.LoadScriptFile("tests_typedArray.js");
testObjects = testObjects.concat(tests_typedArray);

WScript.LoadScriptFile("tests_array.js");
testObjects = testObjects.concat(tests_array);

// test Map and Set lookups; they should behave like normal objects, keys and values ignored
testObjects = testObjects.concat([
    {
        get: function () {
            var em = new Map();

            var m1 = new Map();
            m1.set('a', 1);

            var m3 = new Map();
            m3.set('b', 2);
            m3.set('c', 3);
            m3.set('d', 4);

            var emwp = new Map();
            emwp.x = 5;
            emwp.y = { a: 6 };

            var m1wp = new Map();
            m1wp.set('e', 7);
            m1wp.x = 8;
            m1wp.y = { a: 9 };

            var m3wp = new Map();
            m3wp.set('f', 10);
            m3wp.set('g', 11);
            m3wp.set('h', 12);
            m3wp.x = 13;
            m3wp.y = { a: 14 };

            var root = {
                em: em,
                m1: m1,
                m3: m3,
                emwp: emwp,
                m1wp: m1wp,
                m3wp: m3wp
            };

            return root;
        },
        lookups: [
            ["em"],
            ["m1"],
            ["m3"],
            ["emwp"],
            ["m1wp"],
            ["m3wp"],

            ["em", "x"],
            ["m1", "x"],
            ["m3", "x"],
            ["emwp", "x"],
            ["m1wp", "x"],
            ["m3wp", "x"],

            ["em", "y", "a"],
            ["m1", "y", "a"],
            ["m3", "y", "a"],
            ["emwp", "y", "a"],
            ["m1wp", "y", "a"],
            ["m3wp", "y", "a"],
        ]
    },
    {
        get: function () {
            var es = new Set();

            var s1 = new Set();
            s1.add(1);

            var s3 = new Set();
            s3.add(2);
            s3.add(3);
            s3.add(4);

            var eswp = new Set();
            eswp.x = 5;
            eswp.y = { a: 6 };

            var s1wp = new Set();
            s1wp.add(7);
            s1wp.x = 8;
            s1wp.y = { a: 9 };

            var s3wp = new Set();
            s3wp.add(10);
            s3wp.add(11);
            s3wp.add(12);
            s3wp.x = 13;
            s3wp.y = { a: 14 };

            var root = {
                es: es,
                s1: s1,
                s3: s3,
                eswp: eswp,
                s1wp: s1wp,
                s3wp: s3wp
            };

            return root;
        },
        lookups: [
            ["es"],
            ["s1"],
            ["s3"],
            ["eswp"],
            ["s1wp"],
            ["s3wp"],

            ["es", "x"],
            ["s1", "x"],
            ["s3", "x"],
            ["eswp", "x"],
            ["s1wp", "x"],
            ["s3wp", "x"],

            ["es", "y", "a"],
            ["s1", "y", "a"],
            ["s3", "y", "a"],
            ["eswp", "y", "a"],
            ["s1wp", "y", "a"],
            ["s3wp", "y", "a"],
        ]
    }
]);

var stock_lookups = [
    ["prop1"],
    ["prop1", "prop2"]
];
function getTestLookups(test, testObj)
{
    var lookups;
    try{
        lookups = test.lookups;
        if (!lookups) {
            var keys = Object.keys(testObj);
            lookups = [];
            for (i in keys) {
                lookups.push(new Array(keys[i]));
            }
        }
    } catch(e) {
    }

    if (!lookups) {
        lookups = [];
    }
    return lookups;
}

function getLookupLabel(parts)
{
    var label = parts.join(".");
    if (label.length < 30)
    {
        label += "                              ".slice(label.length);
    }
    return label;
}

for (p in testObjects)
{
    var test = testObjects[p];
    var obj = getTestObject(test);
    var lookups = getTestLookups(test, obj);

    var dump = fmt.stringify(obj);
    echo(dump);

    guarded_call(function(){
        var blob = SCA.serialize(obj);

        for(var i in lookups) {
            var parts = lookups[i];
            var res = SCA.lookup(blob, parts);
            if (res instanceof Date || res instanceof Array) {
                res = JSON.stringify(res); // Print implementation independent string
            }
            echo(getLookupLabel(parts), res);
        }

        // Always lookup stock_lookups, but don't display expected "NoProperty" result.
        for(var i in stock_lookups) {
            var parts = stock_lookups[i];
            var res = SCA.lookup(blob, parts);
            if (res !== "NoProperty, CanAdd" && res !== "NoProperty") {
                echo(getLookupLabel(parts), res);
            }
        }
    });

    echo();
}

for (p in testObjects)
{
    var test = testObjects[p];
    var obj = getTestObject(test);
    var lookups = getTestLookups(test, obj);

    var dump = fmt.stringify(obj);
    echo(dump);

    guarded_call(function(){
        var blob = SCA.serialize(obj);

        for(var i in lookups) {
            var parts = lookups[i];
            var res = SCA.lookupEx(blob, parts);
            if (res instanceof Date || res instanceof Array) {
                res = JSON.stringify(res); // Print implementation independent string
            }
            echo(getLookupLabel(parts), res);
        }

        // Always lookup stock_lookups, but don't display expected "NoProperty" result.
        for(var i in stock_lookups) {
            var parts = stock_lookups[i];
            var res = SCA.lookupEx(blob, parts);
            if (res !== "NoProperty, CanAdd" && res !== "NoProperty") {
                echo(getLookupLabel(parts), res);
            }
        }
    });

    echo();
}

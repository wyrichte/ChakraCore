//
// Misc sca tests
//
WScript.LoadScriptFile("sca_lib.js");

//
// This test suite has properties on prototypes
//
Object.defineProperty(Object.prototype, "abc",
    {value: "Should NOT appear in SCA", enumerable:true});
Object.defineProperty(Object.prototype, 5,
    {value: "Should NOT appear in SCA", enumerable:true});
Object.defineProperty(Array.prototype, 6,
    {value: "Should NOT appear in SCA", enumerable:true});

var testObjects = (function(){
    var tests = [
        // An object with a getter property
        {
            get a() { return "propa"; }
        }
    ];

    // A dense array with getter property
    var o = [0,1,2];
    Object.defineProperty(o, 0, {
        get: function() { return "prop0"; },
        enumerable: true
    });
    tests.push(o);

    // A sparse array with getter property
    var o = [0,1,2];
    Object.defineProperties(o, {
        1: {get: function() { return "prop1"; }, enumerable: true},
        2: {set: function() {}, enumerable: true},
        3: {get: function() { return "prop3"; }, enumerable: false},
        4: {set: function() {}, enumerable: false},
    });
    o.length = 100;
    tests.push(o);

    return tests;
})();

function testSCA(obj, silent)
{
    var dump = fmt.stringify(obj);
    if (!silent) {
        echo(dump);
    }

    guarded_call(function(){
        var blob = SCA.serialize(obj);
        if (!silent) {
            fmt.print(blob);
        }

        var obj2 = SCA.deserialize(blob);
        var dump2 = fmt.stringify(obj2);
        var blob2 = SCA.serialize(obj2);

        if (dump !== dump2 || !arrEquals(blob, blob2)) {
            echo("unmatch");
            echo(dump2);
            fmt.print(blob2);
        }
    });

    if (!silent) {
        echo();
    }
}

for (var i = 0; i < testObjects.length; i++)
{
    var test = testObjects[i];
    var obj = getTestObject(test);
    testSCA(obj);
}

// Test cycle resolving, don't output anything if passes
for (var i = 0; i < testObjects.length; i++)
{
    var test = testObjects[i];
    var obj = getTestObject(test);

    var root = {
        a: null,
        b: null
    };
    root.a = obj;
    root.b = obj;

    testSCA(root, true);
}

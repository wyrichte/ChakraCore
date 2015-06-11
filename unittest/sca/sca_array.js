//
// Test sca of typed array
//
WScript.LoadScriptFile("sca_lib.js");

WScript.LoadScriptFile("tests_array.js");
var testObjects = tests_array;

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

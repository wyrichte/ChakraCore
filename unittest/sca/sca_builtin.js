//
// Test sca of builtin objects
//
WScript.LoadScriptFile("sca_lib.js");

WScript.LoadScriptFile("tests_builtin.js");
var testObjects = tests_builtin;

if(WScript.Arguments != undefined && WScript.Arguments.length > 0) {
    // If user args are passed, test with those args
    testObjects = [];
    for(var argNum in WScript.Arguments) {        
        testObjects.push(WScript.Arguments[argNum]);    
    }
}

function testSCA(obj, isNested)
{
    var dump = fmt.stringify(obj);
    echo(dump);

    // If not testing nested, add a property and ensure ignored by SCA
    if (!isNested) {
        obj["ShouldNotSeeThis"] = 1234;
    }

    guarded_call(function(){
        var blob = SCA.serialize(obj);
        fmt.print(blob);

        var obj2 = SCA.deserialize(blob);
        var dump2 = fmt.stringify(obj2);
        var blob2 = SCA.serialize(obj2);

        if (dump !== dump2 || !arrEquals(blob, blob2)) {
            echo("unmatch");
            echo(dump2);
            fmt.print(blob2);
        }
    });

    if (!isNested) {
        delete obj["ShouldNotSeeThis"];
    }
    echo();
}

for (p in testObjects)
{
    var test = testObjects[p];
    var obj = getTestObject(test);

    testSCA(obj);

    // Test cycle resolving with builtin objects
    var root = {
        a: null,
        b: null
    };
    root.a = obj;
    root.b = obj;
    testSCA(root, true);
}

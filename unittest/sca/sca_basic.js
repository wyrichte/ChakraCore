//
// Test sca basics (primitives)
//
WScript.LoadScriptFile("sca_lib.js");

WScript.LoadScriptFile("tests_primitive.js");
var testObjects = tests_primitive;

for (p in testObjects)
{
    var obj = getTestObject(testObjects[p]);
    echo(JSON.stringify(obj));

    guarded_call(function(){
        var blob = SCA.serialize(obj);
        fmt.print(blob);

        var obj2 = SCA.deserialize(blob);
        if (obj !== obj2 && !(obj !== obj && obj2 !== obj2)) {
            echo("unmatch", obj, obj2);
        }
    });

    echo();
}

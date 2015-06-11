//
// Test sca of Map and Set objects
//
WScript.LoadScriptFile("sca_lib.js");

WScript.LoadScriptFile("tests_mapset.js");

for (var p in tests_mapset)
{
    var test = tests_mapset[p];
    var obj = getTestObject(test);

    var dump = fmt.stringify(obj);
    echo(dump);

    // Add a non-enumerable property, which should be ignored by SCA.
    Object.defineProperty(obj, "ShouldNotSee_NonEnumerable", {
        value: 123,
        enumerable: false
    });

    guarded_call(function () {
        var blob = SCA.serialize(obj);
        fmt.print(blob);

        var obj2 = SCA.deserialize(blob);
        var blob2 = SCA.serialize(obj2);
        var dump2 = fmt.stringify(obj2);

        if (!arrEquals(blob, blob2) || dump !== dump2) {
            echo("unmatch");
            echo(dump2);
            fmt.print(blob2);
        }
    });

    echo();
}

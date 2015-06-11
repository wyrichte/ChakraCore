//
// Test sca errors
//
WScript.LoadScriptFile("sca_lib.js");

//
// Test SCA unsupported types: DATA_CLONE_ERROR
//
var tests_error = (function(){
    var tests = [
        function(){},
        new Function("1 + 1"),
    ];

    tests.push(fmt.stringify);
    tests.push(SCA.serialize);

    function getArguments() {
        return arguments;
    }
    tests.push(getArguments("Arguments object"));

    function getProxy() {
        var myProxy = new Proxy({}, {});
        return myProxy;
    }
    tests.push(getProxy);
    // nested
    tests = tests.concat(tests.map(function(test){
        var root = {};
        root.abc = getTestObject(test);
        return root;
    }));

    return tests;
})();

tests_error.forEach(function(test) {
    var obj = getTestObject(test);

    var dump = fmt.stringify(obj);
    echo(dump);

    guarded_call(function(){
        SCA.serialize(obj);
        echo("FAILED: Should have thrown");
    });

    echo();
});

//
// Test corrupted SCA layout data
//
var tests_corrupt = (function(){
    var tests = [];

    function pushTest(name, data) {
        var test = {};
        test.name = name;
        test.data = data;
        tests.push(test);
    }

    // Get a sample SCA format layout
    var getData = (function(){
        var data = SCA.serialize({a: "a"});
        return function() {
            return new Uint8Array(data);
        };
    })();

    var data = getData();
    data[3] = data[3] + 1;
    pushTest("new major version", data);

    var data = getData();
    data[2] = data[2] + 1;
    pushTest("new minor version - should be ok", data);

    var data = getData();
    var dv = new DataView(data.buffer);
    dv.setUint32(4, 0, true);
    pushTest("root SCATypeID is none", data);

    var data = getData();
    var dv = new DataView(data.buffer);
    dv.setUint32(4, 99, true); // SCA_FirstHostObject - 1
    pushTest("root SCATypeID is unknown", data);

    var data = getData();
    var dv = new DataView(data.buffer);
    dv.setUint32(16, 0, true);
    pushTest("property value SCATypeID is none", data);

    var data = getData();
    var dv = new DataView(data.buffer);
    dv.setUint32(16, 99, true);
    pushTest("property value SCATypeID is unknown", data);

    var data = getData();
    var dv = new DataView(data.buffer);
    dv.setUint32(20, -1, true);
    pushTest("string length is property terminator", data);

    var data = getData();
    var dv = new DataView(data.buffer);
    dv.setUint32(20, 1000, true);
    pushTest("string length is longer than existed data", data);

    // Get a truncated copy of sample SCA format layout
    var getTruncatedData = (function(){
        var data = getData();
        return function(byteOffset, byteLength) {
            var view = new Uint8Array(data.buffer, byteOffset, byteLength);
            return new Uint8Array(view); // Return a copy with a new ArrayBuffer
        };
    })();

    var data2 = getTruncatedData(0, 0);
    pushTest("truncated, empty", data2);

    var data2 = getTruncatedData(0, 3);
    pushTest("truncated, incomplete header", data2);

    var data2 = getTruncatedData(0, 4);
    pushTest("truncated, no value", data2);

    var data2 = getTruncatedData(0, 5);
    pushTest("truncated, incomplete SCATypeId", data2);

    var data2 = getTruncatedData(0, 16);
    pushTest("truncated, missing property value", data2);

    var data2 = getTruncatedData(0, 28);
    pushTest("truncated, missing property terminator", data2);

    return tests;
})();

tests_corrupt.forEach(function(test) {
    echo(test.name);
    fmt.print(test.data);

    guarded_call(function(){
        var obj = SCA.deserialize(test.data);
        echo(fmt.stringify(obj));
    });

    guarded_call(function(){
        var result = SCA.lookup(test.data, [["a"]]);
        echo(result);
    });

    echo();
});

//
// SCA tests for typed arrays
//
var tests_uint8clampedarray_rawlen;
var tests_uint8clampedarray = (function(){
    var data = [
        [],
        [0, 1, 2],
        [3, 4, 34, 56],
        [-1, 2, -3, 4, -5, 6, -7]
    ];

    var arrTypes = [
        Uint8ClampedArray
    ];

    var tests = [];

    // Add ArrayBuffers
    data.forEach(function(arr){
        var test = {};
        test.root = new Uint8Array(arr).buffer;
        test.lookups = [["byteLength"]];
        tests.push(test);
    });

    // Add TypedArrays
    arrTypes.forEach(function(type){
        data.forEach(function(arr){
            var test = {};
            test.root = new type(arr);
            test.lookups = [["byteOffset"],["length"]];
            tests.push(test);
        });
    });

    // Add DataViews
    data.forEach(function(arr){
        var test = {};
        test.root = new DataView(new Uint8Array(arr).buffer);
        test.lookups = [["byteOffset"],["byteLength"]];
        tests.push(test);
    });

    // Mark raw TypedArray/DataView entries count. Used for testing named property behavior.
    // All entries so far are single ArrayBuffer/TypedArray/DataView entries.
    tests_uint8clampedarray = tests.length;

    return tests;
})();

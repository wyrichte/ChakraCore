//
// SCA tests for typed arrays
//
var tests_typedArray_rawlen;
var tests_typedArray = (function(){
     var data = [
        [],
        [0, 1, 2],
        [3, 4, 34, 56],
        [-1, 2, -3, 4, -5, 6, -7]
    ]; 
    var arrTypes = [
        Int8Array, Uint8Array, Int16Array, Uint16Array, Int32Array, Uint32Array, Float32Array, Float64Array
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
	
	
    arrTypes.forEach(function(type){
            var test = {};
            test.root = new type(new Array(1<<16));
            test.lookups = [["byteOffset"],["length"]];
            tests.push(test);
        
    });
 	
    // Mark raw TypedArray/DataView entries count. Used for testing named property behavior.
    // All entries so far are single ArrayBuffer/TypedArray/DataView entries.
    tests_typedArray_rawlen = tests.length;

    // A root with TypedArrays sharing ArrayBuffer
    (function(){
        var buf = new ArrayBuffer(16);
        var arr = new Uint8Array(buf);
        for (var i = 0; i < 16; i++) {
            arr[i] = i;
        }
        var arrs = [
            [Int8Array,     0, 1],
            [Uint8Array,    1, 2],
            [Int16Array,    2, 3],
            [Uint16Array,   2, 4],
            [Int32Array,    4, 0],
            [Uint32Array,   0, 3],
            [Float32Array,  4, 3],
            [Float64Array,  8, 1],
            [DataView,      3, 7]
        ];
        var root = {};
        var props = [];
        arrs.forEach(function(arr){
            var ctor = arr[0];
            props.push(new ctor(buf, arr[1], arr[2]));
        });
        addProperties(root, props);
        tests.push(root);
    })();
    return tests;
})();

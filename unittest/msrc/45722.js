// Used to box the argument
function arg() {
    return arg.arguments[0];
}

// Creates/builds/returns a deep-copied array
function fff() {
    // This is created on the stack when fff is JIT'd
    let stack = [];
    // Fill the array with ints
    for (let i = 0; i < 0x100; i++) {
        stack[i] = i;
    }

    let heap = arg(stack);

    // Makes the array a NativeFloatArray, only modifies stack instance
    // Note that only stack instance gets this data (because heap instance is deep copied)
    stack[0x300] = 1.1;

    return heap;
}

// Fill obj_arr with deep-copied arrays
let obj_arr = [];
for (let i = 0; i < 2000; i++) {
    obj_arr[i] = fff();
}

// Add additional segment past original head segment of obj_arr[1333]. Setting to
// object also changes it from NativeFloat to Var array
obj_arr[1333][3000] = {};

// Splice removes and returns all of the elements of obj_arr[1333]. Before fix, Splice
// incorrectly treats the head segment as a heap-allocated segment rather than an inlined
// segment because of the head segments size.
let evil = obj_arr[1333].splice(0, obj_arr[1333].length);

// Free up memory
for (let i = 0; i < obj_arr.length; i++) {
    obj_arr[i] = null;
}

// Use this helper var with String ctor below to force LdElem into the array
var s = "";

// Iterate through evil for UaF
for (let i = 0; i < evil.length; i++) {
    s += new String(evil[i]);
}

print(s.substring(0,0) + "PASSED");

function inlinee() {
    new Error();
    return inlinee.arguments[0];
}

function opt(convert_to_var_array) {
    // To make the in-place type conversion happen, it must have multiple segments
    let stack_arr = []; // JavascriptNativeFloatArray
    stack_arr[10000] = 1.1;
    stack_arr[20000] = 2.2;

    let heap_arr = inlinee(stack_arr);
    convert_to_var_array(heap_arr); // on the final call, it's changed to a Var array because of {}

    stack_arr[10000] = 2.3023e-320;  // but, on the final call, it's still written to as if it's a nativefloat

    // this is where bailout occurs (BailOutOnNotNativeArray), because it's trying to read the float value as a Var
    return heap_arr[10000];
}

function main() {
    for (let i = 0; i < 130; i++) {
        opt(new Function('')); // Prevents inlining
    }

    var x = opt(function(heap_arr) {
        heap_arr[10000] = {}; // ConvertToVarArray
    });
}

main();
print("PASSED");

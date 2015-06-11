//
// Test uncaught exception source info
//
function dummy() {
    // do nothing
}

function funcThrow() {
    dummy();
    throw 123; //10,5
    dummy();
}

dummy();
funcThrow();
dummy();

//
// Test uncaught exception source info
//
function dummy() {
    // do nothing
}

dummy();
var obj = {
    get foo() {    // This needs -Version:3
        dummy();
        throw 123; //12,9
        dummy();
    }
};
obj.foo; //12,1
dummy();

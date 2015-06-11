//
// Test uncaught exception source info
//
function dummy() {
    // do nothing
}

dummy();
var obj = {
    func: function () {
        dummy();
        throw 123; //12,9
        dummy();
    }
};
obj.func();
dummy();

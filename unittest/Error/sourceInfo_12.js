//
// Test uncaught exception source info
//
function dummy() {
    // do nothing
}

dummy();
var obj = {
    apply: Function.prototype.apply
};
obj.apply(); //12,1
dummy();

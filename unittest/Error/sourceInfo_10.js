//
// Test uncaught exception source info
//
function dummy() {
    // do nothing
}

dummy();
var obj = {};
  obj.nosuchfunc(); //10,3
dummy();

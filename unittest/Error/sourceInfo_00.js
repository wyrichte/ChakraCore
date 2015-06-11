//
// Test uncaught exception source info
//
function dummy() {
    // do nothing
}

dummy();
nosuchfunc(); //9,1
dummy();

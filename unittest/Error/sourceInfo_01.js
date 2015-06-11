//
// Test uncaught exception source info
//
function dummy() {
    // do nothing
}

dummy();
  throw 123; //9,3
dummy();

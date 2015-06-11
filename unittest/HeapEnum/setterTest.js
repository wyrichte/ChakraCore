// Tests that the setter of an object dumps with proper rsFlags set (along with default getter).

function setFunction() {
}

var o = {};
Object.defineProperty(o, "foo", { set: setFunction });
Debug.dumpHeap(
    o,
    /*dump log*/ true,
    /*forbaselineCompare*/ true,
    /*rootsOnly*/ false,
    /*returnArray*/ false,
    /*enumFlags = PROFILER_HEAP_ENUM_FLAGS_STORE_RELATIONSHIP_FLAGS*/ 1);
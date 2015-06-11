// Tests that the getter of an object dumps with proper rsFlags set (along with default setter).

function getFunction() {
}

var o = {};
Object.defineProperty(o, "foo", { get: getFunction });
Debug.dumpHeap(
    o,
    /*dump log*/ true,
    /*forbaselineCompare*/ true,
    /*rootsOnly*/ false,
    /*returnArray*/ false,
    /*enumFlags = PROFILER_HEAP_ENUM_FLAGS_STORE_RELATIONSHIP_FLAGS*/ 1);
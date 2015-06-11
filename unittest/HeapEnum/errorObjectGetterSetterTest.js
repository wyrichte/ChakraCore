// Tests that the getter/setter of the error object dumps with a different ID.
// Found in bug #4989.

try {
    // We need to construct a new ReferenceError before throwing in order to delay-initialize ReferenceError.
    // During HeapEnum we can't change Recycler state so at that time we can't initialize any delayed objects.
    var d = new ReferenceError('');
    someUndefinedFunction();
}
catch (ex) {
    var error = ex;
    Debug.dumpHeap(
        error,
        /*dump log*/ true,
        /*forbaselineCompare*/ true,
        /*rootsOnly*/ false,
        /*returnArray*/ false,
        /*enumFlags = PROFILER_HEAP_ENUM_FLAGS_STORE_RELATIONSHIP_FLAGS*/ 1);
}
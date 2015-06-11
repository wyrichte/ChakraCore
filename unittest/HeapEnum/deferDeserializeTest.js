// Tests that defer deserialized functions don't cause
// a crash when being enumerated by heap enum.
// Bug 215818.

function test() {
}

Debug.dumpHeap(null, true, true, false, false, 0);
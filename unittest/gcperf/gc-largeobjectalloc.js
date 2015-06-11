// 
// Micro-benchmark to measure peak working set for large object
// The idea is allocate a large object, and then a second large object
// and then free the first large object, and repeat. This should
// cause fragmentation 
//

function AllocLargeObject(size)
{
    var descriptor = { disposableObjectSize: size, bytesToAllocator: 0, allocationCount: 0 }

    return Debug.createDebugDisposableObject(descriptor);
}

function createMediumObjectWithFragmentation()
{
    var largeObjectSize = 16 * 1024 - 0x2f;
    var mediumObjectSize = 2048 - 0x2f;

    var largeObject = AllocLargeObject(largeObjectSize);
    var mediumObject = AllocLargeObject(mediumObjectSize);

    // Cause the medium object to be the only one alive
    largeObject = null;
    return mediumObject;
}

var mediumObjects = [];
WScript.Echo("Starting test");
for (var i = 0; i < 16 * 4096; i++) {
    mediumObjects.push(createMediumObjectWithFragmentation());
}

CollectGarbage();

WScript.Echo("Ending test");

var memoryInfo = Debug.getWorkingSet();
WScript.Echo("Peak working set: " + memoryInfo.maxWorkingSet);

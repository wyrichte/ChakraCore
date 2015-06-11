// Test disposable objects
// This test allocates a disposable object based on the parameters passed in below
// The disposable object can cause an allocation, either explicitly or by allocation
// 
// An object of type DebugDisposableObject can be created in debug/fretest builds using Debug.createDebugDisposableObject
// This function expects a parameter which is the object descriptor
// The descriptor has the following known properties:
//  - collectOnDispose: 
//	Whether to trigger an in-thread GC when the allocated object is disposed. 
// 	By default, this is false		
//  - allocateLeaf: 
//	Whether the allocated object allocates a leaf object on dispose
//	By default this is true. If this is false, we'll allocate a disposable object on dispose too.
//  - bytesToAllocate: 
//	The size of the object to allocate when the object is disposed.
//	By default, this is 16.
//  - disposableObjectSize: 
//	The size of the disposable object that Debug.createDebugDisposableObject creates. 
// 	By default, this is 8192
//  - allocationCount:
//	The number of allocations to do during dispose.
//	By default, this is 256
//
// In the following case, we allocate 1024 disposable objects. Each object is a large object of size 9192 bytes.
// None of them force an in-thread collection during dispose. 
// Each of them allocates a leaf object of size 256 bytes, and there are 4096 such allocations
// upon dispose
//

var descriptor = { bytesToAllocateOnDispose: 256, allocationCount: 4096 }

WScript.Echo("Starting test");
for (var i = 0; i < 1024; i++) {
    var disposableObject = Debug.createDebugDisposableObject(descriptor);
}

disposableObject = null;
CollectGarbage();

WScript.Echo("Ending test");
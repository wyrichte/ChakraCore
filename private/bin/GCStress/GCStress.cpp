/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/
#include "stdafx.h"
#include "GCStress.h"

void DoVerify(bool value, const char * expr, const char * file, int line)
{
    if (!value)
    {
        wprintf(L"==== FAILURE: '%S' evaluated to false. %S(%d)\n", expr, file, line);
        DebugBreak();
    }
}

// Some constants for the stress test

static const unsigned int stackRootCount = 50;
static const unsigned int globalRootCount = 50;
static const unsigned int implicitRootCount = 50;

static const unsigned int initializeCount = 1000000;

static const unsigned int operationsPerHeapWalk = 1000000;

// Some global variables

// Recycler instance
Recycler * recycler = nullptr;

// TODO, make this configurable on the command line
bool implicitRootsMode = false;
//bool implicitRootsMode = true;

// List of root locations.  These may be on the stack or elsewhere.
WeightedTable<Location> roots;

// Global root locations.  These are pinned (if not null).
RecyclerTestObject * globalRoots[globalRootCount];

// Implicit root locations.  These are allocated using ImplicitRootBit.
// Only enabled in MemProtect mode.
RecyclerTestObject * implicitRoots[implicitRootCount];

// Object creation function table.  Used to randomly create new objects.
typedef RecyclerTestObject * (*ObjectCreationFunc)(void);

WeightedTable<ObjectCreationFunc> objectCreationTable;

// Operation table.  Used to randomly perform heap operations.
typedef void (*Operation)(void);

WeightedTable<Operation> operationTable;

// Not used currently, but keep for now
bool verbose = false;


RecyclerTestObject * CreateNewObject()
{
    // Get a random creation routine from the objectCreationTable
    ObjectCreationFunc creationFunc = objectCreationTable.GetRandomEntry();

    // Invoke it to create the new object
    return creationFunc();
}

Location GetRandomLocation()
{
    Location location = roots.GetRandomEntry();

    while (true)
    {
        // If the current location contains nullptr, we can't walk it.
        // Just return this location.
        RecyclerTestObject * object = location.Get();
        if (object == nullptr)
        {
            return location;
        }

        // Once in a while, just stop walking and return the current location, even though it's not nullptr.
        // We don't want to do this too often, because if we update the location, we'll prune the entire tree
        // underneath it.  So make this relatively rare.
        // (Note, different object mixes may require this to be tuned up/down as appropriate.)
        if (GetRandomInteger(10000) == 0)
        {
            return location;
        }

        // Otherwise, try to walk to a new location on the specified object
        if (!object->TryGetRandomLocation(&location))
        {
            // TryGetRandomLocation failed, e.g. because the object is a leaf object and has no internal locations.
            // Thus we can't walk any further.  Return the location we have.
            return location;
        }
    }
}

void InsertObject()
{
    // Create a new object
    RecyclerTestObject * object = CreateNewObject();
    
    // Walk to a random location in the current object graph
    Location location = GetRandomLocation();

    // If the location is currently null, set the object there
    // If it's not null, do nothing and let the new object be collected.
    if (location.Get() == nullptr)
    {
        location.Set(object);
    }
}

void ReplaceObject()
{
    // Create a new object
    RecyclerTestObject * object = CreateNewObject();
    
    // Walk to a random location in the current object graph
    Location location = GetRandomLocation();

    // Set the new object there unconditionally
    location.Set(object);
}

void DeleteObject()
{
    // Walk to a random location in the current object graph
    Location location = GetRandomLocation();

    // Set it to nullptr
    location.Set(nullptr);
}

void MoveObject()
{
    // Walk to two random locations in the current object graph
    Location location1 = GetRandomLocation();
    Location location2 = GetRandomLocation();

    // Move the reference and delete the old reference.
    RecyclerTestObject * object = location1.Get();
    location1.Set(nullptr);
    location2.Set(object);
}

void CopyObject()
{
    // Walk to two random locations in the current object graph
    Location location1 = GetRandomLocation();
    Location location2 = GetRandomLocation();

    // Copy from the first reference to second.
    RecyclerTestObject * object = location1.Get();
    location2.Set(object);
}

void SwapObjects()
{
    // Walk to two random locations in the current object graph
    Location location1 = GetRandomLocation();
    Location location2 = GetRandomLocation();

    // Swap their references.
    RecyclerTestObject * object = location1.Get();
    location1.Set(location2.Get());
    location2.Set(object);
}

void DoHeapOperation()
{
    // Get a random heap operation routine from the operation table
    Operation operationFunc = operationTable.GetRandomEntry();

    // Invoke it to perform the heap walk
    return operationFunc();
}

void WalkHeap()
{
    RecyclerTestObject::BeginWalk();

    // The roots table always has weight 1 for every entry, so we can walk it directly without hitting duplicates.

    for (unsigned int i = 0; i < roots.GetSize(); i++)
    {
        RecyclerTestObject::WalkReference(roots.GetEntry(i).Get());
    }

    RecyclerTestObject::EndWalk();
}

void BuildObjectCreationTable()
{
    // Populate the object creation func table
    // This defines the set of objects we create and their relative weights
    objectCreationTable.AddWeightedEntry(&LeafObject<1, 50>::New, 1000);
    objectCreationTable.AddWeightedEntry(&ScannedObject<1, 50>::New, 10000);
    objectCreationTable.AddWeightedEntry(&BarrierObject<1, 50>::New, 2000);
    objectCreationTable.AddWeightedEntry(&TrackedObject<1, 50>::New, 2000);

    objectCreationTable.AddWeightedEntry(&LeafObject<51, 1000>::New, 10);
    objectCreationTable.AddWeightedEntry(&ScannedObject<51, 1000>::New, 100);
    objectCreationTable.AddWeightedEntry(&BarrierObject<51, 1000>::New, 20);
    objectCreationTable.AddWeightedEntry(&TrackedObject<51, 1000>::New, 20);
    
    objectCreationTable.AddWeightedEntry(&LeafObject<1001, 50000>::New, 1);
    objectCreationTable.AddWeightedEntry(&ScannedObject<1001, 50000>::New, 10);
    objectCreationTable.AddWeightedEntry(&BarrierObject<1001, 50000>::New, 2);
//    objectCreationTable.AddWeightedEntry(&TrackedObject<1001, 50000>::New, 2);    // Large tracked objects are not supported
}

void BuildOperationTable()
{
    operationTable.AddWeightedEntry(&InsertObject, 5);
    operationTable.AddWeightedEntry(&ReplaceObject, 2);
    operationTable.AddWeightedEntry(&DeleteObject, 2);
    operationTable.AddWeightedEntry(&MoveObject, 5);
    operationTable.AddWeightedEntry(&CopyObject, 5);
    operationTable.AddWeightedEntry(&SwapObjects, 5);
}

void SimpleRecyclerTest()
{
    // Initialize the probability tables for object creation and heap operations.
    BuildObjectCreationTable();
    BuildOperationTable();

    // Construct Recycler instance and use it
    PageAllocator::BackgroundPageQueue backgroundPageQueue;
    IdleDecommitPageAllocator pageAllocator(nullptr, 
        PageAllocatorType::PageAllocatorType_Thread,
        Js::Configuration::Global.flags,
        0 /* maxFreePageCount */, PageAllocator::DefaultMaxFreePageCount /* maxIdleFreePageCount */,
        false /* zero pages */, &backgroundPageQueue);

    try
    {
#ifdef EXCEPTION_CHECK
        // REVIEW: Do we need a stack probe here? We don't care about OOM's since we deal with that below
        AUTO_NESTED_HANDLED_EXCEPTION_TYPE(ExceptionType_DisableCheck);
#endif

        recycler = HeapNewZ(Recycler, nullptr, &pageAllocator, Js::Throw::OutOfMemory, Js::Configuration::Global.flags);

        recycler->Initialize(false /* forceInThread */, nullptr /* threadService */);

#if FALSE
        // TODO: Support EnableImplicitRoots call on Recycler (or similar, e.g. constructor param)
        // Until then, implicitRootsMode support doesn't actually work.
        if (implicitRootsMode)
        {
            recycler->EnableImplicitRoots();
        }
#endif

        wprintf(L"Recycler created, initializing heap...\n");
        
        // Initialize stack roots and add to our roots table        
        RecyclerTestObject * stackRoots[stackRootCount];
        for (unsigned int i = 0; i < stackRootCount; i++)
        {
            stackRoots[i] = nullptr;
            roots.AddWeightedEntry(Location::Scanned(&stackRoots[i]), 1);
        }

        // Initialize global roots and add to our roots table        
        for (unsigned int i = 0; i < globalRootCount; i++)
        {
            globalRoots[i] = nullptr;
            roots.AddWeightedEntry(Location::Rooted(&globalRoots[i]), 1);
        }

        // MemProtect only:
        // Initialize implicit roots and add to our roots table        
        if (implicitRootsMode)
        {
            for (unsigned int i = 0; i < implicitRootCount; i++)
            {
                implicitRoots[i] = nullptr;
                roots.AddWeightedEntry(Location::ImplicitRoot(&implicitRoots[i]), 1);
            }
        }
        
        // Initialize GC heap randomly
        for (unsigned int i = 0; i < initializeCount; i++)
        {
            InsertObject();
        }

        wprintf(L"Initialization complete\n");

        // Do an initial walk
        WalkHeap();

        // Loop, continually doing heap operations, and periodically doing a full heap walk
        while (true)
        {
            for (unsigned int i = 0; i < operationsPerHeapWalk; i++)
            {
                DoHeapOperation();
            }
            
            WalkHeap();

            // Dispose now
            recycler->FinishDisposeObjectsNow<FinishDispose>();
        }
    }
    catch (Js::OutOfMemoryException)
    {
        printf("Error: OOM\n");
    }

    wprintf(L"==== Test completed.\n");
}

//////////////////// End test implementations ////////////////////


//////////////////// Begin program entrypoint ////////////////////

void usage(const WCHAR* self)
{
    wprintf(
        L"usage: [-?|-v] [-js <jscript options from here on>]\n"
        L"  -v\n\tverbose logging\n",
        self);
}

int __cdecl wmain(int argc, __in_ecount(argc) WCHAR* argv[])
{
    int jscriptOptions = 0;

    for (int i = 1; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            if (wcscmp(argv[i], L"-?") == 0)
            {
                usage(argv[0]);
                exit(1);
            }
            else if (wcscmp(argv[i], L"-v") == 0)
            {
                verbose = true;
            }
            else if (wcscmp(argv[i], L"-js") == 0 || wcscmp(argv[i], L"-JS") == 0)
            {
                jscriptOptions = i;
                break;
            }
            else 
            {
                wprintf(L"unknown argument '%s'\n", argv[i]);
                usage(argv[0]);
                exit(1);
            }
        }
        else
        {
            wprintf(L"unknown argument '%s'\n", argv[i]);
            usage(argv[0]);
            exit(1);
        }
    }

    // Parse the rest of the command line as js options
    if (jscriptOptions)
    {
        CmdLineArgsParser parser(null);
        parser.Parse(argc - jscriptOptions, argv + jscriptOptions);
    }

    // Run the actual test
    SimpleRecyclerTest();

    return 0;
}

//////////////////// End program entrypoint ////////////////////

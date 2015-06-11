/********************************************************
*                                                       *
*   Copyright (C) Microsoft. All rights reserved.       *
*                                                       *
********************************************************/

#if !defined(_M_ARM64)
CompileAssert(false)
#endif

struct XDataAllocation sealed  : public SecondaryAllocation
{
    // ---- Methods ----- //
    XDataAllocation() : 
        pdataCount(0)
        , functionTable(NULL)
        , xdataSize(0)
    {}

    RUNTIME_FUNCTION* GetPdataArray() const
    {
        return reinterpret_cast<RUNTIME_FUNCTION*>(address + xdataSize);
    }

    bool IsFreed() const
    {
        return address == null;
    }

    void Free()
    {
        address = null;
        pdataCount = 0;
        functionTable = null;
        xdataSize = 0;
    }

    // ---- Data members ---- //
    ushort pdataCount;                   // ARM requires more than 1 pdata/function
    FunctionTableHandle functionTable;   // stores the handle to the the growable function table
    ushort xdataSize;                    
};

//
// Allocates xdata and pdata entries for ARM architecture on the heap. They are freed when released.
// 
//
class XDataAllocator sealed : public SecondaryAllocator
{
// -------- Private members ---------/
private:
    ushort pdataCount;
    FunctionTableHandle* functionTableHandles;

// --------- Public functions ---------/
public:
    XDataAllocator(BYTE* address, uint size) { }
    
    bool Initialize(_In_ void* segmentStart, _In_ void* segmentEnd) { __debugbreak(); return 0; }
    void Delete() { __debugbreak(); }
    bool Alloc(ULONG_PTR functionStart, DWORD functionSize, ushort pdataCount, ushort xdataSize, SecondaryAllocation* allocation) { __debugbreak(); return 0; }
    void Register(XDataAllocation& allocation, ULONG_PTR functionStart, DWORD functionSize) { __debugbreak(); }
    void Release(const SecondaryAllocation& address) { __debugbreak(); }
    bool CanAllocate() { __debugbreak(); return 0; }
    DWORD GetAllocSize(ushort pdataCount, ushort xdataSize)
    {
        return sizeof(RUNTIME_FUNCTION) * pdataCount + xdataSize;
    }
};

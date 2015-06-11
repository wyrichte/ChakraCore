//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js {

    ///----------------------------------------------------------------------------
    ///
    /// RcObject::RcObject
    ///
    /// RcObject() provides common preinitialization for all reference-counted
    /// objects.  Derived instances should usually create a custom Construct()
    /// method to provide custom construction that may safely thrown exceptions
    /// during construction.
    ///
    ///----------------------------------------------------------------------------

    inline
    RcObject::RcObject()
    {
#if DBG
        AssertMsg(!jn.m_DEBUG_fConstructCpp, "Ensure object was properly preconstructed");        
        jn.m_DEBUG_fConstructCpp = true;
#endif

        //
        // Initialize the reference-count to '1':
        // - This enables circular references to be created and broken during object construction
        //   without the object being destroyed.  Ex: 1 -> 2 -> 1.
        // - Without this, if a cycle was created and broken, the object could start destruction
        //   even before it was constructed.  Ex: 0 -> 1 -> 0!
        // - This requires that the initial object creation uses RcRef<>::Attach() rather than
        //   constructor or "operator =()", as they would also increase the reference count.
        //

        jn.m_cContextRef = 1;
    }


    ///----------------------------------------------------------------------------
    ///
    /// RcObject::Construct
    ///
    /// Construct() provides type-specific initialization that may safely throw
    /// exceptions during construction.
    ///
    ///----------------------------------------------------------------------------

    inline void
    RcObject::Construct()
    {
#if DBG
        AssertMsg(jn.m_DEBUG_fConstructCpp, "Ensure object was properly preconstructed");        
        AssertMsg(jn.m_cContextRef == 1, "Object should only have initial reference count from constructor");
#endif
     }


    ///----------------------------------------------------------------------------
    ///
    /// RcObject::Lock
    ///
    /// Lock() increments the count of references toward this object instance.
    ///
    ///----------------------------------------------------------------------------

    inline void
    RcObject::Lock()
    {
        //
        // An object may only be locked if it is not being destroyed.  Once the C++ destructor is
        // called, no-one may make any strong references to the instance.
        //
        // TODO: Need to add a flag to detect this.  The object will be initially created with a
        // m_cContextRef == 0 and will not be bumped up until someone makes a strong reference.
        //

        jn.m_cContextRef++;
    }


    ///----------------------------------------------------------------------------
    ///
    /// RcObject::Unlock
    ///
    /// Unlock() decrements the count of references toward this object instance.
    /// When this reaches 0, the object is immediately and fully destroyed, before
    /// this method returns.  The object may not be resurrected.
    ///
    ///----------------------------------------------------------------------------

    inline void
    RcObject::Unlock()
    {
        AssertMsg(jn.m_cContextRef > 0, "Should always have an outstanding reference to unlock");

        if (--jn.m_cContextRef == 0)
        {
            Delete();
        }
    }


    ///----------------------------------------------------------------------------
    /// 
    /// RcObject::operator new
    /// 
    /// operator new() is called to allocate memory for and perform C++
    /// initialization for a new object instance.
    /// 
    ///----------------------------------------------------------------------------

    inline void * 
    RcObject::operator new(
        size_t byteSize)                              // Memory size, in bytes
    {
        //
        // Regular 'new' is used to allocate memory for new instances, then initialize the C++
        // object on that memory.
        //

        void * previousAllocation = StandardHeap::Allocate((int) byteSize);

    #if DBG
        byte * rgbVerify = reinterpret_cast<byte *>(previousAllocation);
        for (size_t index = 0; index < byteSize; index++)
        {
            AssertMsg(rgbVerify[index] == 0, "Ensure object header is zero initialized");
        }
    #endif

        return previousAllocation;
    }
     

    ///----------------------------------------------------------------------------
    /// 
    /// RcObject::operator new
    /// 
    /// operator new() provides the C++ "new" keyword with the memory to allocate
    /// for this placement-new object instance.
    /// 
    ///----------------------------------------------------------------------------

    inline void * 
    RcObject::operator new(
        size_t byteSize,                              // Memory size, in bytes
        void* previousAllocation)                         // Previously allocated memory
    {
        //
        // Placement 'new' is used by Array, RcString and ValueType to initialize the C++ object on
        // previously allocated memory:
        // - The previously allocated memory must be zero-initialized for the header.  This is different
        //   than the non-placement-new form(), but it avoids the extra ZeroMemory() in these
        //   specialized usage.
        //

    #if DBG
        byte * rgbVerify = reinterpret_cast<byte *>(previousAllocation);
        for (size_t index = 0; index < byteSize; index++)
        {
            AssertMsg(rgbVerify[index] == 0, "Ensure object header is zero initialized");
        }
    #endif

        return previousAllocation;
    }


    ///----------------------------------------------------------------------------
    /// 
    /// RcObject::operator delete
    /// 
    /// operator delete() frees memory previously allocated using the C++ "new"
    /// keyword for this object instance.
    /// 
    ///----------------------------------------------------------------------------

    inline void __cdecl
    RcObject::operator delete(
        void* allocationToFree)                // Allocation to free
    {
        StandardHeap::Free(allocationToFree);
    }


    ///----------------------------------------------------------------------------
    /// 
    /// RcObject::operator delete
    /// 
    /// operator delete() is the pair for the placement-new form.  This function
    /// should never actually be called.
    /// 
    ///----------------------------------------------------------------------------

    inline void __cdecl
    RcObject::operator delete(
        void* allocationToFree,                // Allocation to free
        void* previousAllocation)              // Previously allocated memory
    {
        AssertMsg(allocationToFree == previousAllocation, "Memory locations should match");
        AssertMsg(false, "RcObjects must be free'd with RcObject::Delete() instead");
    }

} // namespace Js

//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

namespace Js {

    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
    ///
    /// class VectorS
    ///
    ///----------------------------------------------------------------------------

    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::VectorS
    ///
    /// VectorS() initializes a new object instance.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline
    VectorS<T>::VectorS() : m_rgData(null)
    {
        //
        // All elements in VectorS<T> must be at least "sizeof(int)" large.  This is because of the design
        // that stores the size in the element preceeding the data for the array.
        //

        AssertMsg(sizeof(T) >= sizeof(int), "Ensure minimum element size");
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::~VectorS
    ///
    /// ~VectorS() uninitializes the array, removing all items from the array
    /// and freeing all resources.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline
    VectorS<T>::~VectorS()
    {
        RemoveAll();
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::IsEmpty
    ///
    /// IsEmpty() returns whether the array has no items stored in it.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    bool
    VectorS<T>::IsEmpty() const
    {
        return m_rgData == null;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::GetRawData
    ///
    /// GetRawData() returns the real allocated data without checking if the array
    /// has actually allocated data.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void *
    VectorS<T>::GetRawData() const
    {
        //
        // Blindly return the size
        //

        AssertMsg(m_rgData != null, "Array must be allocated if not checking");
        return &m_rgData[-1];
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::GetRawDataOrNull
    ///
    /// GetRawDataOrNull() returns the real allocated data, or null if no data
    /// has been allocated.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void *
    VectorS<T>::GetRawDataOrNull() const
    {
        //
        // Need to check if array is allocated
        //

        if (m_rgData != null)
        {
            return (void *) (&m_rgData[-1]);
        }
        else
        {
            return null;
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::SetRawSize
    ///
    /// SetRawSize() directly stores the number of items in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::SetRawSize(
        int cNewItems)                          // Number of items
    {
        //
        // Store the size before the array data.  This function should only be called when an array
        // is allocated (and thus have a non-zero size).
        //

        AssertMsg(cNewItems > 0, "Must specify a positive number of items");
        AssertMsg(m_rgData != null, "Must allocate range to set number of items");

        int * pnSize = (int *) GetRawData();
        *pnSize = cNewItems;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::GetSize
    ///
    /// GetSize() returns the number of items stored in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline int
    VectorS<T>::GetSize() const
    {
        if (m_rgData != null)
        {
            int * pnSize = (int *) GetRawData();
            int itemCount = *pnSize;
            AssertMsg(itemCount >= 1, "Must have at least one item");
            return itemCount;
        }
        else
        {
            return 0;
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::SetSize
    ///
    /// SetSize() grows or shrinks the array to the new number of items.  If the
    /// array is grown, the new slots are zero-initialized.  If the array is
    /// shrunk, the excess items are removed.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::SetSize(
        int cNewItems)                          // New number of items
    {
        AssertMsg(cNewItems >= 0, "Must have valid size");

        int cSize = GetSize();
        if (cSize == cNewItems)
        {
            return;
        }

        if (cNewItems == 0)
        {
            //
            // Remove all items from the array.
            //

            RemoveAll();
        }
        else
        {
            //
            // Check if making the array smaller and need to destruct the objects we are getting
            // rid of.
            //

            if (cNewItems < cSize)
            {
                AssertMsg(m_rgData != null, "Should have data allocated");
                for(int i = cNewItems; i < cSize; i++)
                {
                    m_rgData[i].~T();
                }
            }


            //
            // Resize the array.
            //

            Resize(cNewItems);
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::Add
    ///
    /// Add() appends a given item to the end of the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline int
    VectorS<T>::Add(
        const T & t)                            // Item to add
    {
        int idxAdd = GetSize();
        Resize(idxAdd + 1);
        SetAtIndex(idxAdd, t);

        return idxAdd;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::InsertAt
    ///
    /// InsertAt() injects the given item at the given position in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::InsertAt(
        int itemIndex,                            // Index to insert at
        const T & t)                            // Item to add
    {
        AssertMsg((itemIndex <= GetSize()) && (itemIndex >= 0), "Check index");


        //
        // Need to increase the size by one and may need to shift everything down.
        //

        int idxAdd = GetSize();
        Resize(idxAdd + 1);
        int cbMove = (idxAdd - itemIndex) * sizeof(T);
        if (cbMove > 0)
        {
            memmove(&m_rgData[itemIndex + 1], &m_rgData[itemIndex], cbMove);
        }

        SetAtIndex(itemIndex, t);
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::Remove
    ///
    /// Remove() removes the first occurrence of the given item from the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::Remove(
        const T & t)                            // Item to remove
    {
        //
        // Find the first occurrence of the item.
        //

        int itemIndex = Find(t);
        if (itemIndex >= 0)
        {
            //
            // Remove the item.
            //

            RemoveAt(itemIndex);
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::RemoveAt
    ///
    /// RemoveAt() removes the item at the specified slot in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::RemoveAt(int itemIndex)
    {
        int itemCount = GetSize();
        AssertMsg((itemIndex < itemCount) && (itemIndex >= 0) && (itemCount >= 0), "Ensure valid index");


        //
        // Destroy the item.
        //

        m_rgData[itemIndex].~T();


        //
        // Remove the item slot.
        //

        itemCount--;
        if (itemCount > 0)
        {
            //
            // Found the element, so we need to splice it out of the array.  We can not just
            // Realloc() the buffer b/c we need to slide all property data after this down one.
            // This means that we have to allocate a new buffer.  If we are unable to allocate a
            // temporary buffer, we can go ahead and just use the existing buffer, but we won't be
            // able to free any memory.
            //

            if (itemIndex < itemCount)
            {
                memmove((void *) &m_rgData[itemIndex], (void *) &m_rgData[itemIndex + 1], (itemCount - itemIndex) * sizeof(T));
            }

            T * rgNewData = (T *) Js::StandardHeap::ReallocateNoZero(GetRawData(), (itemCount + 1) * sizeof(T));
            m_rgData = &rgNewData[1];

            SetRawSize(itemCount);
        }
        else
        {
            //
            // No remaining items, so free all resources.
            //

            Js::StandardHeap::Free(GetRawData());
            m_rgData = null;
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::RemoveAll
    ///
    /// RemoveAll() removes all items from the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::RemoveAll()
    {
        if (m_rgData != null)
        {
            //
            // Destroy all objects.
            //

            int itemCount = GetSize();
            for(int i = 0; i < itemCount; i++)
            {
                m_rgData[i].~T();
            }


            //
            // Free all resources.
            //

            Js::StandardHeap::Free(GetRawData());
            m_rgData = null;
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::operator[]
    ///
    /// operator[] returns the item at the specified slot.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    __forceinline T &
    VectorS<T>::operator[] (int itemIndex) const
    {
        Assert((itemIndex >= 0) && (itemIndex < GetSize()));
        return m_rgData[itemIndex];
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::GetBuffer
    ///
    /// GetBuffer() returns the underlying storage data for the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    __forceinline T *
    VectorS<T>::GetBuffer() const
    {
        return m_rgData;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::SetAtIndex
    ///
    /// SetAtIndex() stores the given item at the specified index in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::SetAtIndex(int itemIndex, const T & t)
    {
        AssertMsg((itemIndex >= 0) && (itemIndex < GetSize()), "Check index");
        placement_copynew(&m_rgData[itemIndex], T, t);
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::Find
    ///
    /// Find() searches for and returns the first occurrence of item in the array.
    /// If the item is not found, it returns -1.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline int
    VectorS<T>::Find(const T & t) const
    {
        int itemCount = GetSize();
        for(int i = 0; i < itemCount; i++)
        {
            if (m_rgData[i] == t)
            {
                return i;
            }
        }

        return -1;  // not found
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::Extract
    ///
    /// Extract() transfers the contents from the given array into this array.
    /// After the transfer, the source is emptied.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::Extract(
        VectorS<T> * parSrc)                   // Array to extract contents from
    {
        m_rgData = parSrc->m_rgData;
        parSrc->m_rgData = null;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::Resize
    ///
    /// Resize() changes the size of the array to a non-zero number of elements.
    ///
    /// NOTE: This function has been specifically written for the VectorS<T>
    /// class and has slightly different behavior that VectorF<T>::Resize().
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::Resize(
        int cNewItems)                          // New number of items
    {
        AssertMsg(cNewItems > 0, "Must have non-zero and positive number of items");
        AssertMsg((cNewItems + 1) < cNewItems, "Overflowing max array size");


        //
        // Resize the array and store the new size.
        //

        T * rgNewData = (T *) Js::StandardHeap::ReallocateNoZero(GetRawDataOrNull(), (cNewItems + 1) * sizeof(T));
        m_rgData = &rgNewData[1];
        SetRawSize(cNewItems);
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorS<T>::SetRange(int idxStart, int idxStop, const T & t);
    ///
    /// SetRange() sets a range [idxStart, idxStop) of items to value t.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorS<T>::SetRange(
        int idxStart,                           // Start index of the range
        int idxStop,                            // End index of the range
        const T & t)                            // value to set
    {
        AssertMsg((idxStart < GetSize()) && (idxStart >= 0), "Must specify a valid start index");
        AssertMsg((idxStop <= GetSize()) && (idxStop >= 0), "Must specify a valid end index");
        AssertMsg(idxStart <= idxStop, "Start index must be <= End index");

        for (int index = idxStart; index < idxStop; index++)
        {
            m_rgData[index] = t;
        }
    }


    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------
    ///
    /// class VectorF
    ///
    ///----------------------------------------------------------------------------
    ///----------------------------------------------------------------------------

    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::VectorF
    ///
    /// VectorF() initializes a new object instance.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline
    VectorF<T>::VectorF()
    {
        //
        // Start off using the lookaside pool.
        //

        m_rgData    = reinterpret_cast<T *>(m_rgbSmall);
        m_cAlloc    = k_cSmall;
        m_cUsed     = 0;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::~VectorF
    ///
    /// ~VectorF() uninitializes the array, removing all items from the array
    /// and freeing all resources.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline
    VectorF<T>::~VectorF()
    {
        RemoveAll();
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::UsingLookaside
    ///
    /// UsingLookaside() returns whether the array is using the lookaside pool for
    /// storing a small number of array items.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline bool
    VectorF<T>::UsingLookaside() const
    {
        return m_rgData == reinterpret_cast<const T *>(m_rgbSmall);
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::IsEmpty
    ///
    /// IsEmpty() returns whether the array has no items stored in it.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline bool
    VectorF<T>::IsEmpty() const
    {
        //
        // VectorF may have a non-null m_rgData but a m_cUsed if only Add() and Remove() are used,
        // treating the array like a stack.  Therefore, we must use m_cUsed to determine if the
        // array is "empty".  To free all memory allocated by the array, use RemoveAll().
        //

        return m_cUsed <= 0;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::GetSize
    ///
    /// GetSize() returns the number of items stored in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline int
    VectorF<T>::GetSize() const
    {
        return m_cUsed;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::SetSize
    ///
    /// SetSize() grows or shrinks the array to the new number of items.  If the
    /// array is grown, the new slots are zero-initialized.  If the array is
    /// shrunk, the excess items are removed.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorF<T>::SetSize(
        int cNewItems)                          // New number of items
    {
        AssertMsg(cNewItems >= 0, "Must have valid size");


        //
        // Check if any changes.
        //

        if (cNewItems == m_cUsed)
        {
            return;
        }


        //
        // Check if removing all items.
        //

        if (cNewItems == 0)
        {
            RemoveAll();
            return;
        }


        //
        // Check if making the array smaller, so need to destruct the objects we are getting rid of.
        //

        if (cNewItems < m_cUsed)
        {
            for(int i = cNewItems; i < m_cUsed; i++)
            {
                m_rgData[i].~T();
            }
        }


        //
        // Resize the array to the new size.
        //

        int cOldItems = m_cUsed;
        Resize(cNewItems);


        //
        // Zero initialize the new data slots.
        //

        if (cNewItems > cOldItems)
        {
            memset(&m_rgData[cOldItems], 0, (cNewItems - cOldItems) * sizeof(T));
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::Add
    ///
    /// Add() appends a given item to the end of the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline int
    VectorF<T>::Add(
        const T & t)                            // Item to add
    {
        //
        // Grow the array.
        //

        int idxAdd = m_cUsed;
        Resize(m_cUsed + 1);


        //
        // Store the new item at the end of the array.
        //

        SetAtIndex(idxAdd, t);
        return idxAdd;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::InsertAt
    ///
    /// InsertAt() injects the given item at the given position in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorF<T>::InsertAt(
        int itemIndex,                            // Index to insert at
        const T & t)                            // Item to insert
    {
        AssertMsg((itemIndex <= m_cUsed) && (itemIndex >= 0), "Must specify a valid index");

        //
        // Grow the array.
        //

        Resize(m_cUsed + 1);


        //
        // Move everything after the insertion down one item.
        //

        int cbMove = (m_cUsed - itemIndex - 1) * sizeof(T);
        if (cbMove > 0)
        {
            memmove(&m_rgData[itemIndex + 1], &m_rgData[itemIndex], cbMove);
        }


        //
        // Store the new item.
        //

        SetAtIndex(itemIndex, t);
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::Remove
    ///
    /// Remove() removes the first occurrence of the given item from the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorF<T>::Remove(const T & t)
    {
        //
        // Find the first occurrence
        //

        int itemIndex = Find(t);
        if (itemIndex >= 0)
        {
            //
            // Remove the occurrence.
            //

            RemoveAt(itemIndex);
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::RemoveAt
    ///
    /// RemoveAt() removes the item at the specified slot in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorF<T>::RemoveAt(
        int itemIndex)                            // Index to remove at
    {
        AssertMsg((itemIndex < m_cUsed) && (itemIndex >= 0), "Must specify a valid index");
            

        //
        // Destroy the item.
        //

        m_rgData[itemIndex].~T();


        //
        // Move everything after the deletion up one item.
        //

        if (itemIndex != (m_cUsed - 1))
        {
            memmove((void *) &m_rgData[itemIndex], (void *) &m_rgData[itemIndex + 1],
                    (m_cUsed - itemIndex - 1) * sizeof(T));
        }

        if (m_cUsed == 1)
        {
            //
            // Removing the last item in the array.
            //

            RemoveAll();
        }
        else
        {
            //
            // Resize the array.  This should always succeed because we're only making the array
            // smaller.
            //

            Resize(m_cUsed - 1);
        }
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::RemoveAll
    ///
    /// RemoveAll() removes all items from the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorF<T>::RemoveAll()
    {
        //
        // Destroy all items.
        //

        //
        // JNTODO: Should not need to call C++ destructor on primitive types, such as 'byte'.  The
        // C++ compiler is not optimizing out the empty loop.
        //

        for(int i = 0; i < m_cUsed; i++)
        {
            m_rgData[i].~T();
        }


        //
        // Free allocated memory.
        //

        if (!UsingLookaside())
        {
            Js::StandardHeap::Free(m_rgData);
        }


        //
        // Reset to use lookaside pool.
        //

        m_rgData    = reinterpret_cast<T *>(m_rgbSmall);
        m_cAlloc    = k_cSmall;
        m_cUsed     = 0;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::operator[]
    ///
    /// operator[] returns the item at the specified slot.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    __forceinline T &
    VectorF<T>::operator[] (int itemIndex) const
    {
        AssertMsg((itemIndex < m_cUsed) && (itemIndex >= 0), "Must specify a valid index");
        return m_rgData[itemIndex];
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::GetBuffer
    ///
    /// GetBuffer() returns the underlying storage data for the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    __forceinline T *
    VectorF<T>::GetBuffer() const
    {
        return m_rgData;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::SetAtIndex
    ///
    /// SetAtIndex() stores the given item at the specified index in the array.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorF<T>::SetAtIndex(int itemIndex, const T & t)
    {
        AssertMsg((itemIndex < m_cUsed) && (itemIndex >= 0), "Must specify a valid index");
        placement_copynew(&m_rgData[itemIndex], T, t);
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::Find
    ///
    /// Find() searches for and returns the first occurrence of item in the array.
    /// If the item is not found, it returns -1.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline int
    VectorF<T>::Find(const T & t) const
    {
        //
        // Search for the first occurrence of the item.
        //

        for (int i = 0; i < m_cUsed; i++)
        {
            if (m_rgData[i] == t)
            {
                return i;
            }
        }


        //
        // Instance was not found.
        //

        return -1;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::Resize()
    ///
    /// Resize() changes the size of the array to accommodate the specified number
    /// of stored items.  After calling this function, the underlying data members
    /// have all been updated for the new item count.
    ///
    /// Notes:
    /// - If the array is growing, the caller is responsible for zeroing the new
    ///   data slots.
    /// - If the array is shrinking, the caller is responsible for first destroying
    ///   the data members.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorF<T>::Resize(
        int cNewItems)                          // New number of used items in array.
    {
        AssertMsg(m_rgData != null, "Always should point to buffer (begin)");
        AssertMsg(cNewItems > 0, "Should not call Resize() when removing all items");
        AssertMsg(m_cAlloc >= m_cUsed, "Ensure legal sizes");

        bool fOldUsingLookaside = UsingLookaside();
        bool fNewUsingLookaside = cNewItems <= k_cSmall;
        AssertMsg((!fOldUsingLookaside) || (m_cUsed <= k_cSmall),
                "If currently using lookaside, old size must fit");

        if (fOldUsingLookaside && fNewUsingLookaside)
        {
            //
            // No change.
            //
        }
        else if (!fOldUsingLookaside && fNewUsingLookaside)
        {
            //
            // Changing to a smaller size, so redirect to point to our lookaside array.
            //

            AssertMsg(m_rgData != reinterpret_cast<T *>(m_rgbSmall), "Should not already be using lookaside");
            js_memcpy_s(m_rgbSmall, sizeof(T[k_cSmall]), m_rgData, cNewItems * sizeof(T));
            Js::StandardHeap::Free(m_rgData);

            m_cAlloc    = k_cSmall;
            m_rgData    = reinterpret_cast<T *>(m_rgbSmall);
        }
        else
        {
            //
            // Determine the allocated array size.  We use a power of two policy to actually grow
            // and shrink the array.  We continue computing the new array size until the specified
            // number of items will actually fit.
            //

            AssertMsg(!fNewUsingLookaside, "Should not start using lookaside");
            AssertMsg(m_cAlloc > 0, "Must always have some storage allocated");

            int cNewAlloc = m_cAlloc;
            while (true)
            {
                if (cNewItems < (cNewAlloc / 2))
                {
                    //
                    // We can shrink the array and reclaim some memory.
                    //

                    cNewAlloc /= 2;
                }
                else if (cNewItems > cNewAlloc)
                {
                    //
                    // We need to grow the array.
                    //

                    cNewAlloc *= 2;
                }
                else
                {
                    //
                    // Allocation doesn't need to change.
                    //

                    break;
                }
            }

            AssertMsg(cNewItems > k_cSmall, "Should not need to use lookaside pool");
            AssertMsg(cNewItems <= cNewAlloc, "Should have ample storage for new items");


            //
            // Check if we need to allocate / reallocate memory for the array storage.
            //

            if (cNewAlloc != m_cAlloc)
            {
                T * rgNewData = (T *) Js::StandardHeap::ReallocateNoZero(
                        fOldUsingLookaside ? null : m_rgData, cNewAlloc * sizeof(T));

                if (fOldUsingLookaside)
                {
                    js_memcpy_s(rgNewData, cNewAlloc * sizeof(T), m_rgbSmall, sizeof(m_rgbSmall));
                }

                m_cAlloc    = cNewAlloc;
                m_rgData    = rgNewData;
            }
        }

        AssertMsg(m_rgData != null, "Always should point to buffer (end)");


        //
        // Successfully allocated storage, so update the number of stored items.
        //

        m_cUsed = cNewItems;
    }


    ///----------------------------------------------------------------------------
    ///
    /// VectorF<T>::SetRange
    ///
    /// SetRange() sets a range [idxStart, idxStop) of items to value t.
    ///
    ///----------------------------------------------------------------------------

    template <class T>
    inline void
    VectorF<T>::SetRange(
        int idxStart,                           // Start index of the range
        int idxStop,                            // End index of the range
        const T & t)                            // value to
    {
        AssertMsg((idxStart < m_cUsed) && (idxStart >= 0), "Must specify a valid start index");
        AssertMsg((idxStop <= m_cUsed) && (idxStop >= 0), "Must specify a valid end index");
        AssertMsg(idxStart <= idxStop, "Start index must be <= End index");

        switch (sizeof(T))
        {
            case 1:
            {
                memset(m_rgData + idxStart, (byte) t, (idxStop - idxStart));
                break;
            }

            case 2:
            {
                wmemset(m_rgData + idxStart, (wchar_t) t, (idxStop - idxStart));
                break;
            }

            default:
            {
                for (int index = idxStart; index < idxStop; index++)
                {
                    m_rgData[index] = t;
                }
            }
        }
    }

} // namespace Js

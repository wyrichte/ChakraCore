//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

// Contains a class which will provide a uint32 array which can grow dynamically
// It behaves almost same as regex::List<> except it has less members, is customized for being used in SmallSpanSequence of FunctionBody


#pragma once

#ifdef DIAG_MEM
extern int listFreeAmount;
#endif

namespace JsUtil
{
    template <class TValue, class TAllocator>
    class GrowingArray
    {
    public:
        typedef typename AllocatorInfo<TAllocator, TValue>::AllocatorType AllocatorType;
        static GrowingArray* Create(int _length);

        GrowingArray(AllocatorType* allocator, int _length)
            : buffer(nullptr),
            alloc(allocator),
            count(0),
            length(_length)
        {
            EnsureArray();
        }

        ~GrowingArray()
        {
            if (buffer != nullptr)
            {
                AllocatorFree(alloc, (TypeAllocatorFunc<AllocatorType, int>::GetFreeFunc()), buffer, length * sizeof(TValue));
            }
        }

        TValue ItemInBuffer(int index)
        {
            if (index < 0 || index >= count)
            {
                return 0;
            }

            return buffer[index];
        }

        void ItemInBuffer(int index, TValue item)
        {
            EnsureArray();
            Assert(index < count);
            buffer[index] = item;
        }

        void Add(TValue item)
        {
            EnsureArray();
            buffer[count] = item;
            count++;
        }

        int Count() const { return count; }
        void SetCount(int _count) { count = _count; }
        int GetLength() const { return length; }
        TValue* GetBuffer() const { return buffer; }

        GrowingArray * Clone()
        {
            GrowingArray * pNewArray = AllocatorNew(AllocatorType, alloc, GrowingArray, alloc, length);
            pNewArray->count = count;
            if (buffer)
            {
                pNewArray->buffer = AllocateArray<AllocatorType, TValue, false>(
                    TRACK_ALLOC_INFO(alloc, TValue, AllocatorType, 0, length),
                    TypeAllocatorFunc<AllocatorType, TValue>::GetAllocFunc(),
                    length);
                js_memcpy_s(pNewArray->buffer, sizeof(TValue) * length, buffer, (size_t)(sizeof(TValue)*length));
            }

            return pNewArray;
        }
    private:

        TValue* buffer;
        int count;
        int length;
        AllocatorType* alloc;

        void EnsureArray()
        {
            if (buffer == nullptr)
            {
                buffer = AllocateArray<AllocatorType, TValue, false>(
                    TRACK_ALLOC_INFO(alloc, TValue, AllocatorType, 0, length),
                    TypeAllocatorFunc<AllocatorType, TValue>::GetAllocFunc(),
                    length);
                count = 0;
            }
            else if (count == length)
            {
                //int newLength = length + increment;
                int newLength = (length + 1) << 1;
                TValue * newbuffer = AllocateArray<AllocatorType, TValue, false>(
                    TRACK_ALLOC_INFO(alloc, TValue, AllocatorType, 0, newLength),
                    TypeAllocatorFunc<AllocatorType, TValue>::GetAllocFunc(),
                    newLength);
                js_memcpy_s(newbuffer, newLength * sizeof(TValue), buffer, sizeof(TValue)*length);
#ifdef DIAG_MEM
                listFreeAmount += length;
#endif
                if (length != 0)
                {
                    AllocatorFree(alloc, (TypeAllocatorFunc<AllocatorType, int>::GetFreeFunc()), buffer, length * sizeof(TValue));
                }
                length = newLength;
                buffer = newbuffer;
            }
        }
    };
    typedef GrowingArray<uint32, HeapAllocator> GrowingUint32HeapArray;
}

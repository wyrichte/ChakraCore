//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#pragma once

#define FOREACH_BITSET_IN_FIXEDBV(index, bv) \
{ \
    BVIndex index; \
    for(JsUtil::FBVEnumerator _bvenum = bv->BeginSetBits(); \
        !_bvenum.End(); \
        _bvenum++) \
    { \
        index = _bvenum.GetCurrent(); \

#define NEXT_BITSET_IN_FIXEDBV              }}


class BVFixed
{
// Data
protected:
    BVIndex       len;
    BVUnit      data[];

private:
    BVFixed(BVFixed * initBv);
    BVFixed(BVIndex length, bool initialSet = false);
    void ClearEnd();

// Creation Factory
public:

    template <typename TAllocator>
    static  BVFixed *       New(TAllocator* alloc, BVFixed * initBv);

    template <typename TAllocator>
    static  BVFixed *       New(BVIndex length, TAllocator* alloc, bool initialSet = false);
    
    template <typename TAllocator>
    static  BVFixed *       NewNoThrow(BVIndex length, TAllocator* alloc, bool initialSet = false);

    template <typename TAllocator>
    void                    Delete(TAllocator * alloc);

    // For preallocated memory
    static size_t           GetAllocSize(BVIndex length);
    void Init(BVIndex length);

// Implementation
protected:    
            void            AssertRange(BVIndex i) const;
            void            AssertBV(const BVFixed * bv) const;

    static  BVIndex         WordCount(BVIndex length);
    
    const   BVUnit *        BitsFromIndex(BVIndex i) const;
            BVUnit *        BitsFromIndex(BVIndex i);
    const   BVUnit *        BeginUnit() const;
            BVUnit *        BeginUnit();
    const   BVUnit *        EndUnit() const;
            BVUnit *        EndUnit();
    

    template<class Fn>    
    __inline void for_each(const BVFixed *bv2, const Fn callback)
    {
        //... this assert may not be valid. what should be the semantic
        AssertMsg(this->len == bv2->len, "Fatal: The 2 bitvectors should have had the same length.");

        BVUnit *        i;
        const BVUnit *  j;

        for(i  =  this->BeginUnit(), j = bv2->BeginUnit();
            i !=  this->EndUnit() ; 
            i++, j++)
        {
            (i->*callback)(*j);
        }
    }

// Methods
public:

    void Set(BVIndex i)
    {
        AssertRange(i);
        this->BitsFromIndex(i)->Set(BVUnit::Offset(i));
    }
    
    void Clear(BVIndex i)
    {
        AssertRange(i);
        this->BitsFromIndex(i)->Clear(BVUnit::Offset(i));
    }
    
    void Compliment(BVIndex i)
    {
        AssertRange(i);
        this->BitsFromIndex(i)->Complement(BVUnit::Offset(i));
    }
    
    BOOLEAN Test(BVIndex i) const
    {
        AssertRange(i);
        return this->BitsFromIndex(i)->Test(BVUnit::Offset(i));
    }
    
    BOOLEAN         operator[](BVIndex i) const;

    BVIndex         GetNextBit(BVIndex i) const;

    BOOLEAN         TestAndSet(BVIndex i);   
    BOOLEAN         TestAndClear(BVIndex i);

    void            OrComplimented(const BVFixed * bv);
    void            Or(const BVFixed *bv);
    uint            DiffCount(const BVFixed* bv) const;
    void            And(const BVFixed *bv);   
    void            Minus(const BVFixed *bv);
    void            Copy(const BVFixed *bv);
    void            CopyBits(const BVFixed * bv, BVIndex i);
    void            ComplimentAll();
    void            SetAll();
    void            ClearAll();

    BVIndex         Count() const;
    BVIndex         Length() const;
    JsUtil::FBVEnumerator BeginSetBits();

    BVIndex         WordCount() const;
    bool            IsAllClear() const;
    BVUnit* GetData() const
    {
        return (BVUnit*)data;
    }
#if DBG_DUMP
    void            Dump() const;
#endif
};

template <typename TAllocator>
BVFixed * BVFixed::New(TAllocator * alloc, BVFixed * initBv)
{    
    BVIndex length = initBv->Length();
    BVFixed *result = AllocatorNewPlus(TAllocator, alloc, sizeof(BVUnit) * BVFixed::WordCount(length), BVFixed, initBv);
    return result;
}

template <typename TAllocator>
BVFixed * BVFixed::New(BVIndex length, TAllocator * alloc, bool initialSet)
{    
    BVFixed *result = AllocatorNewPlus(TAllocator, alloc, sizeof(BVUnit) * BVFixed::WordCount(length), BVFixed, length, initialSet);
    return result;
}

template <typename TAllocator>
BVFixed * BVFixed::NewNoThrow(BVIndex length, TAllocator * alloc, bool initialSet)
{    
    BVFixed *result = AllocatorNewNoThrowPlus(TAllocator, alloc, sizeof(BVUnit) * BVFixed::WordCount(length), BVFixed, length, initialSet);
    return result;
}

template <typename TAllocator>
void BVFixed::Delete(TAllocator * alloc)
{    
    AllocatorDeletePlus(TAllocator, alloc, sizeof(BVUnit) * this->WordCount(), this);
}

template <size_t bitCount>
class BVStatic
{
public:
    // Made public to allow for compile-time use
    static const size_t wordCount = ((bitCount - 1) >> BVUnit::ShiftValue) + 1;

// Data
private:
    BVUnit data[wordCount];

public:
    // Break on member changes. We rely on the layout of this class being static so we can
    // use initializer lists to generate collections of BVStatic.
    BVStatic()
    {
        Assert(sizeof(BVStatic<bitCount>) == sizeof(data));
        Assert((void*)this == (void*)&this->data);
    }

// Implementation
private:
    void AssertRange(BVIndex i) const { Assert(i < bitCount); }
    
    const BVUnit * BitsFromIndex(BVIndex i) const { AssertRange(i); return &this->data[BVUnit::Position(i)]; }
    BVUnit * BitsFromIndex(BVIndex i) { AssertRange(i); return &this->data[BVUnit::Position(i)]; }
    
    const BVUnit * BeginUnit() const { return &this->data[0]; }
    BVUnit * BeginUnit() { return &this->data[0]; }
    
    const BVUnit * EndUnit() const { return &this->data[wordCount]; }
    BVUnit * EndUnit() { return &this->data[wordCount]; }    

    template<class Fn>    
    __inline void for_each(const BVStatic *bv2, const Fn callback)
    {
        BVUnit *        i;
        const BVUnit *  j;

        for(i  =  this->BeginUnit(), j = bv2->BeginUnit();
            i !=  this->EndUnit() ; 
            i++, j++)
        {
            (i->*callback)(*j);
        }
    }

    template<class Fn>    
    static bool MapUntil(const BVStatic *bv1, const BVStatic *bv2, const Fn callback)
    {
        const BVUnit *  i;
        const BVUnit *  j;

        for(i  =  bv1->BeginUnit(), j = bv2->BeginUnit();
            i !=  bv1->EndUnit() ; 
            i++, j++)
        {
            if (!callback(*i, *j))
            {
                return false;
            }
        }
        return true;
    }

    void ClearEnd()
    {
        uint offset = BVUnit::Offset(bitCount);
        if (offset != 0)
        {
            this->data[wordCount - 1].And((1 << offset) - 1);
        }
    }

// Methods
public:
    void Set(BVIndex i)
    {
        AssertRange(i);
        this->BitsFromIndex(i)->Set(BVUnit::Offset(i));
    }
    
    void Clear(BVIndex i)
    {
        AssertRange(i);
        this->BitsFromIndex(i)->Clear(BVUnit::Offset(i));
    }
    
    void Compliment(BVIndex i)
    {
        AssertRange(i);
        this->BitsFromIndex(i)->Complement(BVUnit::Offset(i));
    }

    BOOLEAN Equal(BVStatic<bitCount> const * o)
    {      
        return MapUntil(this, o, [](BVUnit const& i, BVUnit const &j) { return i.Equal(j); });
    }
    
    BOOLEAN Test(BVIndex i) const
    {
        AssertRange(i);
        return this->BitsFromIndex(i)->Test(BVUnit::Offset(i));
    }

    BOOLEAN TestAndSet(BVIndex i)
    {
        AssertRange(i);
        return _bittestandset((long *)this->data, (long) i);
    }
    
    BOOLEAN TestIntrinsic(BVIndex i) const
    {
        AssertRange(i);
        return _bittest((long *)this->data, (long) i);
    }

    BOOLEAN TestAndSetInterlocked(BVIndex i)
    {
        AssertRange(i);
        return _interlockedbittestandset((long *)this->data, (long) i);
    }
    
    BOOLEAN TestAndClear(BVIndex i)
    {
        AssertRange(i);
        BVUnit * bvUnit = this->BitsFromIndex(i);
        BVIndex offset = BVUnit::Offset(i);
        BOOLEAN bit = bvUnit->Test(offset);
        bvUnit->Clear(offset);
        return bit;
    }

    void OrComplimented(const BVStatic * bv) { this->for_each(bv, &BVUnit::OrComplimented); ClearEnd(); }
    void Or(const BVStatic *bv) { this->for_each(bv, &BVUnit::Or); }
    void And(const BVStatic *bv) { this->for_each(bv, &BVUnit::And); }    
    void Minus(const BVStatic *bv) { this->for_each(bv, &BVUnit::Minus); }
    
    void Copy(const BVStatic *bv) { js_memcpy_s(&this->data[0], wordCount * sizeof(BVUnit), &bv->data[0], wordCount * sizeof(BVUnit)); }

    void SetAll() { memset(&this->data[0], -1, wordCount * sizeof(BVUnit)); ClearEnd(); }
    void ClearAll() { memset(&this->data[0], 0, wordCount * sizeof(BVUnit)); }

    void ComplimentAll()
    {
        for (BVIndex i = 0; i < wordCount; i++)
        {
            this->data[i].ComplimentAll();
        }
        
        ClearEnd();    
    }

    BVIndex Count() const
    {
        BVIndex sum = 0;
        for (BVIndex i = 0; i < wordCount; i++)
        {
            sum += this->data[i].Count();
        }
        
        Assert(sum <= bitCount);
        return sum;
    }
    
    BVIndex Length() const
    {
        return bitCount;
    }
    
    JsUtil::FBVEnumerator   BeginSetBits() { return JsUtil::FBVEnumerator(this->BeginUnit(), this->EndUnit()); }

    BVIndex GetNextBit(BVIndex i) const
    {
        AssertRange(i);

        const BVUnit * chunk = BitsFromIndex(i);
        BVIndex base = BVUnit::Floor(i);

        BVIndex offset = chunk->GetNextBit(BVUnit::Offset(i));
        if (-1 != offset)
        {
            return base + offset;
        }

        while (++chunk != this->EndUnit())
        {
            base += BVUnit::BitsPerWord;        
            offset = chunk->GetNextBit();
            if (-1 != offset)
            {
                return base + offset;
            }
        }

       return BVInvalidIndex;
    }

    const BVUnit * GetRawData() const { return data; }

    template <size_t rangeSize>
    BVStatic<rangeSize> * GetRange(BVIndex startOffset)
    {
        AssertRange(startOffset);
        AssertRange(startOffset + rangeSize - 1);

        // Start offset and size must be word-aligned
        Assert(BVUnit::Offset(startOffset) == 0);
        Assert(BVUnit::Offset(rangeSize) == 0);

        return (BVStatic<rangeSize> *)BitsFromIndex(startOffset);
    }

    BOOLEAN TestRange(const BVIndex index, uint length) const
    {
        AssertRange(index);
        AssertRange(index + length - 1);

        const BVUnit * bvUnit = BitsFromIndex(index);
        uint offset = BVUnit::Offset(index);

        if (offset + length <= BVUnit::BitsPerWord)
        {
            // Bit range is in a single word
            return bvUnit->TestRange(offset, length);
        }

        // Bit range spans words.
        // Test the first word, from start offset to end of word
        if (!bvUnit->TestRange(offset, (BVUnit::BitsPerWord - offset)))
        {
            return FALSE;
        }

        bvUnit++;
        length -= (BVUnit::BitsPerWord - offset);
        
        // Test entire words until we are at the last word
        while (length >= BVUnit::BitsPerWord)
        {
            if (!bvUnit->IsFull())
            {
                return FALSE;
            }

            bvUnit++;
            length -= BVUnit::BitsPerWord;
        }

        // Test last word (unless we already ended on a word boundary)
        if (length > 0)
        {
            if (!bvUnit->TestRange(0, length))
            {
                return FALSE;
            }
        }

        return TRUE;
    }

    void SetRange(const BVIndex index, uint length)
    {
        AssertRange(index);
        AssertRange(index + length - 1);

        BVUnit * bvUnit = BitsFromIndex(index);
        uint offset = BVUnit::Offset(index);

        if (offset + length <= BVUnit::BitsPerWord)
        {
            // Bit range is in a single word
            return bvUnit->SetRange(offset, length);
        }

        // Bit range spans words.
        // Set the first word, from start offset to end of word
        bvUnit->SetRange(offset, (BVUnit::BitsPerWord - offset));

        bvUnit++;
        length -= (BVUnit::BitsPerWord - offset);
        
        // Set entire words until we are at the last word
        while (length >= BVUnit::BitsPerWord)
        {
            bvUnit->SetAll();

            bvUnit++;
            length -= BVUnit::BitsPerWord;
        }

        // Set last word (unless we already ended on a word boundary)
        if (length > 0)
        {
            bvUnit->SetRange(0, length);
        }
    }

    void ClearRange(const BVIndex index, uint length)
    {
        AssertRange(index);
        AssertRange(index + length - 1);

        BVUnit * bvUnit = BitsFromIndex(index);
        uint offset = BVUnit::Offset(index);

        if (offset + length <= BVUnit::BitsPerWord)
        {
            // Bit range is in a single word
            return bvUnit->ClearRange(offset, length);
        }

        // Bit range spans words.
        // Clear the first word, from start offset to end of word
        bvUnit->ClearRange(offset, (BVUnit::BitsPerWord - offset));

        bvUnit++;
        length -= (BVUnit::BitsPerWord - offset);
        
        // Set entire words until we are at the last word
        while (length >= BVUnit::BitsPerWord)
        {
            bvUnit->ClearAll();

            bvUnit++;
            length -= BVUnit::BitsPerWord;
        }

        // Set last word (unless we already ended on a word boundary)
        if (length > 0)
        {
            bvUnit->ClearRange(0, length);
        }
    }

    bool IsAllClear()
    {      
        for (BVIndex i = 0; i < wordCount; i++)
        {
            if (!this->data[i].IsEmpty())
            {
                return false;
            }
        }

        return true;
    }

#if DBG_DUMP
    void Dump() const
    {
        bool hasBits = false;
        Output::Print(L"[  ");
        for (BVIndex i = 0; i < wordCount; i++)
        {
            hasBits = this->data[i].Dump(i * BVUnit::BitsPerWord, hasBits);
        }
        Output::Print(L"]\n");
    }
#endif
};





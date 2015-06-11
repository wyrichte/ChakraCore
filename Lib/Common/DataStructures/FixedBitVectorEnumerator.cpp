//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "StdAfx.h"

namespace JsUtil
{
    FBVEnumerator::FBVEnumerator(BVUnit * iterStart, BVUnit * iterEnd):
        icur(iterStart), iend(iterEnd),
        curOffset(0)
    { 
        if(this->icur != this->iend)
        {
            this->curUnit = *iterStart;
            this->MoveToNextBit();
        }
    }

    void
    FBVEnumerator::MoveToValidWord()
    {
        while(curUnit.IsEmpty())
        {
            this->icur++;
            if(this->icur == this->iend)
            {
                return;
            }
            else
            {
                this->curUnit    = *this->icur;
                this->curOffset += BVUnit::BitsPerWord;
            }
        }
    }

    void
    FBVEnumerator::MoveToNextBit()
    {
        if(curUnit.IsEmpty())
        {
            this->curOffset = BVUnit::Floor(curOffset);                
            this->MoveToValidWord();
            if(this->End())
            {
                return;
            }
        }

        BVIndex i = curUnit.GetNextBit();
        AssertMsg(BVInvalidIndex != i, "Fatal Exception. Error in Bitvector implementation");

        curOffset = BVUnit::Floor(curOffset) + i ;
        curUnit.Clear(i);
    }

    void
    FBVEnumerator::operator++(int)
    {
        AssertMsg(this->icur != this->iend, "Iterator past the end of bit stream");
        this->MoveToNextBit();
    }

    BVIndex
    FBVEnumerator::GetCurrent() const
    {
        return this->curOffset;
    }

    bool
    FBVEnumerator::End() const
    {
        return this->icur == this->iend;
    }
}
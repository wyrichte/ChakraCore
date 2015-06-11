//---------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved.
//----------------------------------------------------------------------------

#pragma once

namespace Js
{
    class EHBailoutData
    {
    public:
        int32 nestingDepth;
        int32 catchOffset; 
        EHBailoutData * parent;
        EHBailoutData * child;

    public:
        EHBailoutData() : nestingDepth(-1), catchOffset(0), parent(nullptr), child(nullptr) {}
        EHBailoutData(int32 nestingDepth, int32 catchOffset, EHBailoutData * parent)
        {
            this->nestingDepth = nestingDepth;
            this->catchOffset = catchOffset;
            this->parent = parent;
            this->child = nullptr;
        }
    };
}
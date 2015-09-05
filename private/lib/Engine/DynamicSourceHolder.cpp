//----------------------------------------------------------------------------
// Copyright (C) Microsoft. All rights reserved. 
//----------------------------------------------------------------------------

#include "EnginePch.h"

namespace Js
{
    void DynamicSourceHolder::TryMapSource()
    {
        AssertMsg(this->sourceMapper != nullptr, "Source mapper is nullptr, means the object has been finalized.");
        AssertMsg(!this->isSourceMapped, "Source is already mapped!");

        HRESULT hr =  this->sourceMapper->MapSourceCode((BYTE**)&this->mappedSource, &this->mappedSourceByteLength);
        switch(hr)
        {
        case S_OK:
            this->isSourceMapped = true;
            AssertMsg(this->mappedSource != nullptr, "MapSourceCode returned S_OK, but we have an unmapped source.");
            break;
        case E_OUTOFMEMORY:
            Js::JavascriptError::ThrowOutOfMemoryError(nullptr);
        case E_INVALIDARG:
            AssertMsg(false, "We should not be getting an E_INVALIDARG here, both of the arguments above should be valid.");
            break;
        default:
            AssertMsg(false, "MapSourceCode failed, check hr above.");
            break;
        }
    }

    void DynamicSourceHolder::UnMapSource()
    {
        AssertMsg(this->isSourceMapped, "Source isn't mapped!");
        AssertMsg(this->mappedSource != nullptr, "Mapped source is nullptr.");
        AssertMsg(this->sourceMapper != nullptr, "Source mapper is nullptr, means the object has been finalized.");

        this->sourceMapper->UnmapSourceCode();

        this->isSourceMapped = false;
        this->mappedSource = nullptr;
        this->mappedSourceByteLength = 0;
    }

    void DynamicSourceHolder::Finalize(bool isShutdown)
    {
        if (isSourceMapped)
        {
            UnMapSource();
        }

        sourceMapper->Release();
        sourceMapper = nullptr;
    }
}